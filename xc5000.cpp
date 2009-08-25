/*-
 * Copyright 2009 Jason Harmening
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

#include <unistd.h>
#include <sys/errno.h>
#include "tuner_firmware.h"
#include "xc5000.h"

using namespace std;

#define XC5000_FW_KEY "XC5000_FW"
#define XC5000_SOURCE_KEY "XC5000_SOURCE"

xc5000::xc5000(
   tuner_config &config, 
   tuner_device &device,
   uint32_t ifreq_hz,
   xc5000_reset_callback reset_cb, 
   void *reset_arg,
   int &error)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     avb_driver(config, device),
     m_ifreq_hz(ifreq_hz),
     m_fw_loaded(false),
     m_reset_cb(reset_cb),
     m_reset_arg(reset_arg)
{
   static const uint16_t xc5000_fw_not_loaded = 0x2000;
   static const uint16_t xc5000_fw_loaded     = 0x1388;
   if (error)
   {
      
      return;
   }
   uint16_t id = 0;
   error = read_reg(XC5000_REG_PRODUCT_ID, id);
   if (error)
   {
      return;
   }
   if (id == xc5000_fw_loaded)
   {
      m_fw_loaded = true;
   }
   else if (id != xc5000_fw_not_loaded)
   {
      LIBTUNERLOG << "xc5000: warning: bogus product ID " << id << endl;
   }
}

int xc5000::read_reg(xc5000_read_reg_t reg, uint16_t &data)
{
   uint8_t buf[2];
   buf[0] = (reg >> 8) & 0xFF;
   buf[1] = reg & 0xFF;
   int error = m_device.write(buf, sizeof(buf));
   if (!error)
   {
      error = m_device.read(buf, sizeof(buf));
   }
   if (!error)
   {
      data = (buf[0] << 8) | buf[1];
   }
   return error;
}
      
int xc5000::write_reg(xc5000_write_reg_t reg, uint16_t data)
{
   uint8_t buf[4];
   buf[0] = (reg >> 8) & 0xFF;
   buf[1] = reg & 0xFF;
   buf[2] = (data >> 8) & 0xFF;
   buf[3] = data & 0xFF;
   int error = m_device.write(buf, sizeof(buf));
   uint16_t busy = 0, elapsed = 0;
   while (!error && (elapsed < 1000))
   {
      error = read_reg(XC5000_REG_BUSY, busy);
      if (busy == 0)
      {
         return error;
      }
      usleep(10000);
      elapsed += 10;
   }
   return ETIMEDOUT;
}

int xc5000::load_firmware(void)
{
   const char *fwfile = m_config.get_string(XC5000_FW_KEY);
   if (fwfile == NULL)
   {
      LIBTUNERERR << "xc5000: Firmware file not configured" << endl;
      return ENOENT;
   }
   int error = 0;
   tuner_firmware fw(m_config, fwfile, error);
   if (error)
   {
      LIBTUNERERR << "xc5000: Unable to create firmware image" << endl;
      return error;
   }
   if (m_fw_loaded && fw.up_to_date())
   {
      DIAGNOSTIC(LIBTUNERLOG << "xc5000: NOT updating firmware" << endl)
      return 0;
   }
   LIBTUNERLOG << "xc5000: Loading firmware..." << endl;
   uint8_t *fwdata = reinterpret_cast<uint8_t*>(fw.buffer());
   size_t offset = 0;
   while (!error && (offset < (fw.length() - 1)))
   {
      uint16_t header = (fwdata[offset] << 8) | fwdata[offset + 1];
      offset += 2;
      if (header == 0xFFFF)
      {
         break;
      }
      else if (header == 0x0)
      {
         if (m_reset_cb != NULL)
         {
            error = m_reset_cb(*this, m_reset_arg);
         }
      }
      else if (header & 0x8000)
      {
         usleep((header & 0x7FFF) * 1000);
      }
      else
      {
         if (header > (fw.length() - offset))
         {
            LIBTUNERERR << "xc5000: firmware segment length " << header << " at offset " << offset 
               << " extends beyond end of file" << endl;
            error = EINVAL;
         }
         error = m_device.write(fwdata + offset, header);
         offset += header;
      }
   }
   if (!error)
   {
      m_fw_loaded = true;
      fw.update();
   }
   LIBTUNERLOG << "xc5000: Finished" << endl;
   return error;
}

int xc5000::init(void)
{
   int error = load_firmware();
   if (!error)
   {
      error = write_reg(XC5000_REG_INIT, 0);
   }
   usleep(100000);
   return error;
}

int xc5000::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = init();
   if (error)
   {
      return error;
   }
   xc5000_source_t source = XC5000_SOURCE_AIR;
   switch (channel.modulation)
   {
      case DVB_MOD_VSB_8:
      case DVB_MOD_VSB_16:
      case DVB_MOD_OFDM:
         source = XC5000_SOURCE_AIR;
         break;
      case DVB_MOD_QAM_64:
      case DVB_MOD_QAM_256:
      case DVB_MOD_QAM_AUTO:
         source = XC5000_SOURCE_CABLE;
         break;
      default:
         return EINVAL;
   }
   error = set_source(source);
   if (error)
   {
      return error;
   }
   uint16_t audio_mode_reg = 0x00C0, video_mode_reg = 0;
   int32_t freq_offset = 0;
   switch (channel.bandwidth_hz)
   {
      case 6000000:
         video_mode_reg = 0x8002;
         freq_offset = -1750000;
         break;
      case 7000000:
         video_mode_reg = 0x8007;
         break;
      case 8000000:
         video_mode_reg = 0x800B;
         break;
      default:
         return EINVAL;
   }
   error = write_reg(XC5000_REG_VIDEO_MODE, video_mode_reg);
   if (!error)
   {
      error = write_reg(XC5000_REG_AUDIO_MODE, audio_mode_reg);
   }
   if (!error)
   {
      error = write_reg(XC5000_REG_OUTPUT_FREQ, 
         (uint16_t)(((m_ifreq_hz / 1000) * 1024) / 1000));
   }
   if (!error)
   {
      error = set_frequency((uint32_t)(channel.frequency_hz + freq_offset));
   }
   return error;
}

int xc5000::set_frequency(uint32_t frequency_hz)
{
   
   if ((frequency_hz < 1000000) ||
       (frequency_hz > 1023000000))
   {
      return EINVAL;
   }
   return write_reg(XC5000_REG_INPUT_FREQ, (uint16_t)(frequency_hz / 15625));
}

int xc5000::set_channel(const avb_channel &channel)
{
   int error = init();
   if (error)
   {
      return error;
   }
   xc5000_source_t source = XC5000_SOURCE_AIR;
   error = set_source(source);
   if (error)
   {
      return error;
   }
   uint16_t audio_mode_reg = 0, video_mode_reg = 0;
   switch (channel.video_format)
   {
      case AVB_VIDEO_FMT_NTSC_M:
      case AVB_VIDEO_FMT_NTSC_N:
      case AVB_VIDEO_FMT_NTSC_J:
      case AVB_VIDEO_FMT_PAL_M:
      case AVB_VIDEO_FMT_PAL_N:
      case AVB_VIDEO_FMT_PAL_NC:
         video_mode_reg = 0x8020;
         switch (channel.audio_format)
         {
            case AVB_AUDIO_FMT_A2:
            case AVB_AUDIO_FMT_A2_SAP:
               audio_mode_reg = 0x0600;
               break;
            case AVB_AUDIO_FMT_EIAJ:
            case AVB_AUDIO_FMT_EIAJ_SAP:
               audio_mode_reg = 0x0440;
               break;
            case AVB_AUDIO_FMT_BTSC:
            case AVB_AUDIO_FMT_BTSC_SAP:
               audio_mode_reg = 0x0400;
               break;
            default:
               audio_mode_reg = 0x0478;
               break;
         }
         break;
      case AVB_VIDEO_FMT_PAL_B:
      case AVB_VIDEO_FMT_PAL_G:
         video_mode_reg = 0x8049;
         switch (channel.audio_format)
         {
            case AVB_AUDIO_FMT_A2:
            case AVB_AUDIO_FMT_A2_SAP:
               audio_mode_reg = 0x0A00;
               break;
            case AVB_AUDIO_FMT_NICAM:
            case AVB_AUDIO_FMT_NICAM_SAP:
               audio_mode_reg = 0x0C04;
               break;
            default:
               video_mode_reg = 0x8059;
               audio_mode_reg = 0x0878;
               break;
         }
         break;
      case AVB_VIDEO_FMT_PAL_I:
         video_mode_reg = 0x8009;
         switch (channel.audio_format)
         {
            case AVB_AUDIO_FMT_NICAM:
            case AVB_AUDIO_FMT_NICAM_SAP:
               audio_mode_reg = 0x1080;
               break;
            default:
               audio_mode_reg = 0x0E78;
               break;
         }
         break;
      case AVB_VIDEO_FMT_PAL_D:
      case AVB_VIDEO_FMT_PAL_D1:
      case AVB_VIDEO_FMT_PAL_K:
         video_mode_reg = 0x8009;
         switch (channel.audio_format)
         {
            case AVB_AUDIO_FMT_A2:
            case AVB_AUDIO_FMT_A2_SAP:
               audio_mode_reg = 0x1600;
               break;
            case AVB_AUDIO_FMT_NICAM:
            case AVB_AUDIO_FMT_NICAM_SAP:
               audio_mode_reg = 0x0E80;
               break;
            default:
               audio_mode_reg = 0x1478;
               break;
         }
         break;
      case AVB_VIDEO_FMT_SECAM_D:
      case AVB_VIDEO_FMT_SECAM_K:
      case AVB_VIDEO_FMT_SECAM_K1:
         video_mode_reg = 0x8009;
         switch (channel.audio_format)
         {
            case AVB_AUDIO_FMT_A2:
            case AVB_AUDIO_FMT_A2_SAP:
               audio_mode_reg = 0x1200;
               break;
            default:
               audio_mode_reg = 0x1478;
               break;
         }
         break;
      case AVB_VIDEO_FMT_SECAM_L:
         video_mode_reg = 0x0009;
         audio_mode_reg = 0x8E82;
         break;
      case AVB_VIDEO_FMT_SECAM_LC:
         video_mode_reg = 0x4009;
         audio_mode_reg = 0x8E82;
         break;
      case AVB_VIDEO_FMT_NONE:
         switch (channel.audio_format)
         {
            case AVB_AUDIO_FMT_FM_MONO:
            case AVB_AUDIO_FMT_FM_MONO_NON_USA:
            case AVB_AUDIO_FMT_FM_MONO_USA:
            case AVB_AUDIO_FMT_FM_STEREO:
            case AVB_AUDIO_FMT_FM_STEREO_NON_USA:
            case AVB_AUDIO_FMT_FM_STEREO_USA:
               video_mode_reg = 0x9002;
               audio_mode_reg = 0x0208;
               break;
            default:
               return EINVAL;
         }
      default:
         return EINVAL;
   }
   error = write_reg(XC5000_REG_VIDEO_MODE, video_mode_reg);
   if (!error)
   {
      error = write_reg(XC5000_REG_AUDIO_MODE, audio_mode_reg);
   }
   if (!error)
   {
      error = set_frequency((uint32_t)(channel.frequency_hz));
   }
   return error;
}

int xc5000::set_source(xc5000_source_t &source)
{
   const char *src = m_config.get_string(XC5000_SOURCE_KEY);
   if (src != NULL)
   {
      if (strcasecmp(src, "air") == 0)
      {
         source = XC5000_SOURCE_AIR;
      }
      else if (strcasecmp(src, "cable") == 0)
      {
         source = XC5000_SOURCE_CABLE;
      }
      else
      {
         LIBTUNERERR << "xc5000: Warning: Unrecogized signal source setting " << src << endl;
      }
   }
   return write_reg(XC5000_REG_SIGNAL_SOURCE, source);
}

int xc5000::start(uint32_t timeout_ms)
{
   uint32_t time_slept = 0;
   int error = 0;
   do
   {
      uint16_t locked = 0;
      error = read_reg(XC5000_REG_LOCK, locked);
      if (error || (locked == 1))
      {
         return error;
      }
      usleep(50000);
      time_slept += 50;
   }
   while (time_slept < timeout_ms);
   LIBTUNERERR << "xc5000: tuner not locked" << endl;
   return ETIMEDOUT;
}
