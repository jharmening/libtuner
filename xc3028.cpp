/*- 
 * Copyright 2008 Fritz Katz
 * Copyright 2012 Jason Harmening
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/endian.h>
#include <sys/errno.h>
#include <math.h>
#include <new>
#include "tuner_firmware.h"
#include "xc3028.h"

#define XC3028_FW_KEY        "XC3028_FW"
#define XC3028_MIN_FREQ      42000000
#define XC3028_MAX_FREQ      864000000
#define XC3028_DIVIDER       15625

using namespace std;

xc3028::xc3028(
   tuner_config &config,
   tuner_device &device,
   xc3028_reset_callback_t callback,
   void *callback_context,
   int &error)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     avb_driver(config, device),
     m_callback(callback),
     m_callback_context(callback_context),
     m_firmware(NULL),
     m_base_fws(NULL),
     m_num_base_fws(0),
     m_dvb_fws(NULL),
     m_num_dvb_fws(0),
     m_avb_fws(NULL),
     m_num_avb_fws(0),
     m_scode_fws(NULL),
     m_num_scode_fws(0),
     m_main_fw_offset(0),
     m_current_base(NULL),
     m_current_dvb(NULL),
     m_current_avb(NULL),
     m_current_scode(NULL),
     m_firmware_ver(0),
     m_base_flags(0),
     m_dvb_flags(0),
     m_avb_flags(0),
     m_scode_flags(0),
     m_scode_ifreq_khz(0),
     m_scode_index(0)
{
   if (error)
   {
      return;
   }
   const char *fwfile = m_config.get_string(XC3028_FW_KEY);
   if (fwfile == NULL)
   {
      LIBTUNERERR << "xc3028 firmware file not configured" << endl;
      error = ENOENT;
   }
   m_firmware = new(nothrow) tuner_firmware(config, fwfile, error);
   if (m_firmware == NULL)
   {
      error = ENOMEM;
   }
   if (error || (m_firmware->length() < sizeof(m_firmware_ver)))
   {
      return;
   }
   char *buf = reinterpret_cast<char*>(m_firmware->buffer());
   m_firmware_ver = le16toh(*(reinterpret_cast<uint16_t*>(buf)));
   size_t i = sizeof(m_firmware_ver);
   while ((i + sizeof(fw_section_header)) <= m_firmware->length())
   {
      fw_section_header header = *((fw_section_header*)(buf + i));
      i += sizeof(header);
      header.type = le16toh(header.type);
      if (header.type == XC3028_FW_TYPE_MAIN)
      {
         m_main_fw_offset = i;      
         break;
      }
      header.num_firmwares = le16toh(header.num_firmwares);
      switch (header.type)
      {
         case XC3028_FW_TYPE_BASE:
            m_base_fws = (base_fw_header*)(buf + i);
            m_num_base_fws = header.num_firmwares;
            i += (m_num_base_fws * sizeof(*m_base_fws));
            break;
         case XC3028_FW_TYPE_DVB:
            m_dvb_fws = (dvb_fw_header*)(buf + i);
            m_num_dvb_fws = header.num_firmwares;
            i += (m_num_dvb_fws * sizeof(*m_dvb_fws));
            break;
         case XC3028_FW_TYPE_AVB:
            m_avb_fws = (avb_fw_header*)(buf + i);
            m_num_avb_fws = header.num_firmwares;
            i += (m_num_avb_fws * sizeof(*m_avb_fws));
            break;
         case XC3028_FW_TYPE_SCODE:
            m_scode_fws = (scode_fw_header*)(buf + i);
            m_num_scode_fws = header.num_firmwares;
            i += (m_num_scode_fws * sizeof(*m_scode_fws));
            break;
         default:
            LIBTUNERERR << "xc3028: Unrecognized firmware type " << header.type << " at offset " << i << endl;
            error = EINVAL;
            return;
      }
      if (i >= m_firmware->length())
      {
         LIBTUNERERR << "xc3028: Unexpected end of firmware file" << endl;
         error = EINVAL;
         return;
      }
   }
}

xc3028::~xc3028(void)
{
   reset();
   delete m_firmware;
   m_firmware = NULL;
}

void xc3028::reset(void)
{
   static const uint8_t power_down[] = {0x80, 0x8, 0x0, 0x0};
   printf("xc3028: powering down\n");
   m_device.write(power_down, sizeof(power_down));
   m_current_base = NULL;
}

int xc3028::load_base_fw(uint16_t flags)
{
   flags |= m_base_flags;
   for (uint16_t i = 0; i < m_num_base_fws; ++i)
   {
      uint16_t hdrflags = le16toh(m_base_fws[i].flags);
      if ((hdrflags & flags) == flags)
      {
         if (m_current_base != &m_base_fws[i])
         {
            int error = 0;
            printf("xc3028: Loading base firmware #%hd\n", i); 
            if (m_callback != NULL)
            {
               error = m_callback(XC3028_TUNER_RESET, m_callback_context);
            }
            error = (error ? error : send_firmware(m_base_fws[i].common, "base", i)); 
            if (!error)
            {
               m_current_base = &m_base_fws[i];
               m_current_dvb = NULL;
               m_current_avb = NULL;
               m_current_scode = NULL;
            }
            return error;
         }
         return 0;
      }
   }
   LIBTUNERERR << "xc3028: Unable to find base firmware image for flags " << hex << flags << endl;
   return ENOENT;
}

int xc3028::load_dvb_fw(uint16_t flags, dvb_modulation_t modulation)
{
   uint16_t modulation_mask = ((modulation == DVB_MOD_UNKNOWN) ? 0 : (1 << modulation));
   flags |= m_dvb_flags;
   for (uint16_t i = 0; i < m_num_dvb_fws; ++i)
   {
      uint16_t hdrmodmask = le16toh(m_dvb_fws[i].modulation_mask);
      uint16_t hdrflags = le16toh(m_dvb_fws[i].flags);
      if (((hdrmodmask & modulation_mask) == modulation_mask) &&
          ((hdrflags & flags) == flags))
      {
         m_current_avb = NULL;
         if (m_current_dvb != &m_dvb_fws[i])
         {
            int error = send_firmware(m_dvb_fws[i].common, "DVB", i);
            if (!error)
            {
               m_current_dvb = &m_dvb_fws[i];
               m_current_scode = NULL;
            }
            return error;
         }
         return 0;
      }
   }
   LIBTUNERERR << "xc3028: Unable to find DVB firmware image for flags " << hex << flags << ", modulation " << modulation << endl;
   return ENOENT;
}

int xc3028::load_avb_fw(uint16_t flags, avb_video_fmt_t video_fmt, avb_audio_fmt_t audio_fmt)
{
   uint32_t video_mask = ((video_fmt == AVB_VIDEO_FMT_NONE) ? 0 : (1 << video_fmt));
   uint32_t audio_mask = ((audio_fmt == AVB_AUDIO_FMT_NONE) ? 0 : (1 << audio_fmt));
   flags |= m_avb_flags;
   for (uint16_t i = 0; i < m_num_avb_fws; ++i)
   {
      uint32_t hdrvmask = le32toh(m_avb_fws[i].video_fmt_mask);
      uint32_t hdramask = le32toh(m_avb_fws[i].audio_fmt_mask);
      uint16_t hdrflags = le16toh(m_avb_fws[i].flags);
      if (((hdrvmask & video_mask) == video_mask) &&
          ((hdramask & audio_mask) == audio_mask) &&
          ((hdrflags & flags) == flags))
      {
         m_current_dvb = NULL;
         if (m_current_avb != &m_avb_fws[i])
         {
            printf("xc3028: Loading AVB firmware #%hd\n", i); 
            int error = send_firmware(m_avb_fws[i].common, "AVB", i);
            if (!error)
            {
               m_current_avb = &m_avb_fws[i];
               m_current_scode = NULL;
            }
            return error;
         }
         return 0;
      }
   }
   LIBTUNERERR << "xc3028: Unable to find AVB firmware image for flags " << hex << flags <<
      ", video fmt " << video_fmt << ", audio fmt " << audio_fmt << endl;
   return ENOENT;
}

int xc3028::load_scode_fw(uint16_t flags, uint16_t ifreq_khz)
{
   flags |= m_scode_flags;
   ifreq_khz = (m_scode_ifreq_khz ? m_scode_ifreq_khz : ifreq_khz);
   if (!flags && !ifreq_khz)
   {
      return 0;
   }
   scode_fw_header *fw = NULL;
   for (uint16_t i = 0; i < m_num_scode_fws; ++i)
   {
      uint16_t hdrifreq = le16toh(m_scode_fws[i].ifreq_khz);
      uint16_t hdrflags = le16toh(m_scode_fws[i].flags);
      if ((!ifreq_khz || (hdrifreq == ifreq_khz)) && ((hdrflags & flags) == flags))
      {
         fw = &m_scode_fws[i];
         break;
      }
   }
   if (fw == NULL)
   {
      return ENOENT;
   }
   else if (fw != m_current_scode)
   {
      uint32_t size = le32toh(fw->common.size);
      if (((m_scode_index + 1) * 12) > size)
      {
         return EINVAL;
      }
      uint32_t offset = m_main_fw_offset + le32toh(fw->common.offset) + (m_scode_index * 12);
      static const uint8_t scode_pre[] = {0xA0, 0x00, 0x00, 0x00};
      static const uint8_t scode_post[] = {0x00, 0x8C};
      int error = m_device.write(scode_pre, sizeof(scode_pre));
      error = (error ? error : m_device.write((uint8_t*)m_firmware->buffer() + offset, 12));
      error = (error ? error : m_device.write(scode_post, sizeof(scode_post)));
      if (!error)
      {
         m_current_scode = fw;
      }
      return error;
   }
   return 0;
}

int xc3028::send_firmware(common_fw_header &header, const char *fwtypename, uint16_t fwtypeindex)
{
   int error = 0;
   uint32_t offset = m_main_fw_offset + le32toh(header.offset);
   uint32_t size = le32toh(header.size);
   if ((offset + size) > m_firmware->length())
   {
      LIBTUNERERR << "xc3028: Invalid header for " << fwtypename << " " << fwtypeindex << "; extends beyond end of file" << endl;
      return EINVAL;
   }
   if (offset < m_main_fw_offset)
   {
      LIBTUNERERR << "xc3028: Invalid header for " << fwtypename << " firmware " << fwtypeindex << "; begins before main firmware area" << endl;
      return EINVAL;
   }
   if ((offset + size) < offset)
   {
      LIBTUNERERR << "xc3028: Invalid header for " << fwtypename << " firmware " << fwtypeindex << "; wraps to beginning of file" << endl;
      return EINVAL;
   }
   char *buf = reinterpret_cast<char*>(m_firmware->buffer()) + offset;
   uint32_t i = 0;
   while (!error && (i < (size - 1)))
   {
      uint16_t chunksize = be16toh(*(reinterpret_cast<uint16_t*>(buf + i)));
      i += sizeof(uint16_t);
      switch (chunksize)
      {
         case 0:
            //printf("send_firmware: tuner reset\n");
            if (m_callback != NULL)
            {
               error = m_callback(XC3028_TUNER_RESET, m_callback_context);
            }
            break;
         case 0xFF00:
            //printf("send_firmware: clock reset\n");
            if (m_callback != NULL)
            {
               error = m_callback(XC3028_CLOCK_RESET, m_callback_context);
            }
            break;
         case 0xFFFF:
            printf("send_firmware: exit, bytes written = %d\n", i);
            return 0;
         default:
            if (chunksize > 0xFF00)
            {
               LIBTUNERERR << "xc3028: Unrecognized reset command for " << fwtypename << " firmware " << fwtypeindex << ": " << (chunksize & 0xFF) << endl;
               return EINVAL;
            }
            else if (chunksize & 0x8000)
            {
               //printf("send_firmware: msleep 0x%x\n", chunksize & 0x7FFF);
               usleep((chunksize & 0x7FFF) * 1000);
            }
            else if (((i + chunksize) > size) || ((i + chunksize) < i))
            {
               LIBTUNERERR << "xc3028: Invalid chunk size for " << fwtypename << " firmware " << fwtypeindex << " at offset " << i << endl;
               return EINVAL;
            }
            else
            {
               //printf("send_firmware: %d-byte chunk\n", chunksize);
               uint8_t chunk[64];
               chunk[0] = buf[i++];
               uint16_t remaining = chunksize - 1;
               while (!error && remaining)
               {
                  uint16_t transfer = ((remaining > (sizeof(chunk) - 1)) ? (sizeof(chunk) - 1) : remaining);
                  memcpy(&chunk[1], &buf[i], transfer);
                  error = m_device.write(chunk, transfer + 1);
                  remaining -= transfer;
                  i += transfer;
               }
            }
            break;
      }
   }
   printf("send_firmware: wrote %d bytes, status = %d\n", i, error);
   return error;
}

void xc3028::set_firmware_flags(
   uint16_t base_fw_flags,
   uint16_t dvb_fw_flags,
   uint16_t avb_fw_flags,
   uint16_t scode_fw_flags,
   uint16_t scode_ifreq_khz,
   uint8_t scode_index)
{
   m_base_flags = base_fw_flags;
   m_dvb_flags = dvb_fw_flags;
   m_avb_flags = avb_fw_flags;
   m_scode_flags = scode_fw_flags;
   m_scode_ifreq_khz = scode_ifreq_khz;
   m_scode_index = scode_index;
   if ((m_base_flags & XC3028_BASEFW_MTS) || (m_avb_flags & XC3028_AVBFW_MTS))
   {
      m_base_flags |= XC3028_BASEFW_MTS;
      m_avb_flags |= XC3028_AVBFW_MTS;
   }
}

int xc3028::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   uint16_t base_flags = 0, dvb_flags = 0;
   uint64_t frequency_hz = channel.frequency_hz;
   switch (channel.bandwidth_hz)
   {
      case 6000000:
         dvb_flags |= XC3028_DVBFW_6MHZ;
         frequency_hz -= 1750000;
         break;
      case 7000000:
         dvb_flags |= XC3028_DVBFW_7MHZ;
         base_flags |= XC3028_BASEFW_8MHZ;
         frequency_hz -= 2750000;
         break;
      case 8000000:
         dvb_flags |= XC3028_DVBFW_8MHZ;
         base_flags |= XC3028_BASEFW_8MHZ;
         frequency_hz -= 2750000;
         break;
      default:
         return EINVAL;
   }
   int error = load_base_fw(base_flags);
   error = (error ? error : load_dvb_fw(dvb_flags, channel.modulation));
   load_scode_fw(0, 0);
   error = (error ? error : set_frequency(frequency_hz));
   return error;
}

int xc3028::set_channel(const avb_channel &channel)
{
   uint16_t base_flags;
   switch (channel.video_format)
   {
      case AVB_VIDEO_FMT_NTSC_M:
      case AVB_VIDEO_FMT_NTSC_N:
      case AVB_VIDEO_FMT_NTSC_J:
      case AVB_VIDEO_FMT_NTSC_443:
      case AVB_VIDEO_FMT_PAL_N:
      case AVB_VIDEO_FMT_PAL_NC:
      case AVB_VIDEO_FMT_PAL_M:
      case AVB_VIDEO_FMT_NONE:
         base_flags = 0; 
         break;
      default:
         base_flags = XC3028_BASEFW_8MHZ; 
         break;
   }
   bool radio = false;
   switch (channel.audio_format)
   {
      case AVB_AUDIO_FMT_FM_MONO:
      case AVB_AUDIO_FMT_FM_MONO_NON_USA:
      case AVB_AUDIO_FMT_FM_MONO_USA:
      case AVB_AUDIO_FMT_FM_STEREO:
      case AVB_AUDIO_FMT_FM_STEREO_NON_USA:
      case AVB_AUDIO_FMT_FM_STEREO_USA:
         base_flags |= XC3028_BASEFW_FM;
         radio = (channel.video_format == AVB_VIDEO_FMT_NONE);
         break;
      default:
         break;
   }
   int error = load_base_fw(base_flags);
   error = (error ? error : load_avb_fw(0, channel.video_format, channel.audio_format));
   load_scode_fw(0, 0);
   //if (!radio)
   //{
      static const uint8_t tv_mode[] = {0x0, 0x0};
      error = (error ? error : m_device.write(tv_mode, sizeof(tv_mode)));
   //}
   error = (error ? error : set_frequency(channel.frequency_hz));
   return error;
}

int xc3028::set_frequency(uint64_t frequency_hz)
{
   if ((frequency_hz < XC3028_MIN_FREQ) || (frequency_hz > XC3028_MAX_FREQ))
   {
      return EINVAL;
   }
   static const uint8_t version_reg[] = {0x0, 0x4};
   uint8_t version[2];
   int error = m_device.transact(version_reg, sizeof(version_reg), version, sizeof(version));
   if (error)
   {
      LIBTUNERERR << "xc3028: Unable to read firmware version: " << strerror(error) << endl;
      return error;
   }
   if (version[1] != (m_firmware_ver >> 8))
   {
      LIBTUNERERR << "xc3028: Warning: Unexpected firmware version; expected " << (m_firmware_ver >> 8) << ", read " << version[1] << endl;
   }

   uint32_t divider = (frequency_hz + (XC3028_DIVIDER / 2)) / XC3028_DIVIDER;
   static const uint8_t freq_cmd[] = {0x80, 0x2, 0x0, 0x0};
   error = m_device.write(freq_cmd, sizeof(freq_cmd));
   if (!error && (m_callback != NULL))
   {
      m_callback(XC3028_CLOCK_RESET, m_callback_context);
   }
   usleep(10000);
   divider = htobe32(divider);
   error = (error ? error : m_device.write((uint8_t*)(&divider), sizeof(divider)));
   usleep(100000);
   return error;
}

void xc3028::stop(void) {}

bool xc3028::is_locked(void)
{
   static const uint8_t lock_reg[] = {0x0, 0x2};
   uint8_t lock[2];
   if (m_device.transact(lock_reg, sizeof(lock_reg), lock, sizeof(lock)) != 0)
   {
      return false;
   }
   printf("xc3028: lock registers 0x%x, 0x%x\n", lock[0], lock[1]);
   return ((lock[0] == 0) && (lock[1] == 1));
}

int xc3028::start(uint32_t timeout_ms)
{
   uint32_t time_slept = 0;
   bool locked = false;
   while (!(locked = is_locked()) && (time_slept < timeout_ms))
   {
      usleep(50000);
      time_slept += 50;
   }
   if (!locked)
   {
      LIBTUNERERR << "xc3028: tuner not locked" << endl;
      return ETIMEDOUT;
   }
   return 0;
}

