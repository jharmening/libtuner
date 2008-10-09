/*-
 * xc3028.cpp 
 *  Copyright 2008 Fritz Katz
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
#include "tuner_firmware.h"
#include "xc3028.h"

#define XC3028_FW_KEY "XC3028_FW"
#define XC3028_DELAY_KEY "XC3028_DELAY"
#define XC3028_DEFAULT_DELAY 500000

#define XC3028_MODE_UNKNOWN  0x00
#define XC3028_MODE_VSB      0x06
#define XC3028_MODE_QAM64    0x43
#define XC3028_MODE_QAM256   0x45
#define XC3028_MODE_QAM_AUTO 0x4F

#define XC3028_MIN_FREQ      42000000
#define XC3028_MAX_FREQ      864000000

xc3028::xc3028(
   tuner_config &config,
   tuner_device &device,
   xc3028_callback_t callback,
   void *callback_context,
   int &error,
   uint32_t firmware_flags, // = 0
   uint32_t ifreq_hz /*= 0*/)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     avb_driver(config, device),
     m_callback(callback),
     m_frequency_hz(0),
     m_ifreq_hz(0),
     m_flags(firmware_flags),
     m_format(0),
     m_firmware(NULL),
     m_fw_segs(NULL),
     m_num_segs(0)
{
   if (error)
   {
      return;
   }
   const char *fwfile = m_config.get_string(XC3028_FW_KEY);
   if (fwfile == NULL)
   {
      LIBTUNERERR << "XC3028 firmware file not configured" << endl;
      return ENOENT;
   }
   m_firmware = new tuner_firmware(fwfile, error);
   if (m_firmware == NULL)
   {
      error = ENOMEM;
   }
   if ((error) || (m_firmware->length() > sizeof(m_num_segs)))
   {
      return;
   }
   char *buf = reinterpret_cast<char*>(m_firmware->buffer());
   char *end = buf + m_firmware->length();
   uint16_t segindex = 0;
   m_num_segs = le16toh(*(reinterpret_cast<uint16_t*>(buf)));
   m_fw_segs = new xc3028_fw_header[m_num_segs];
   if (m_fw_segs == NULL)
   {
      error = ENOMEM;
      return;
   }
   buf += sizeof(uint16_t);
   while (((buf + sizeof(xc3028_fw_header)) <= m_firmware->length()) && (segindex < m_num_segs))
   {
      m_fw_segs[segindex].flags = le32toh(*(reinterpret_cast<uint32_t*>(buf)));
      m_fw_segs[segindex].freq_offset_hz = static_cast<uint32_t> (le32toh(*(reinterpret_cast<uint32_t*> (buf))));
      m_fw_segs[segindex].size = le16toh(*(reinterpret_cast<uint16_t*> (buf)));
      buf += (sizeof(uint32_t) + sizeof(int32_t) + sizeof(uint16_t));
      if (m_fw_segs[segindex].flags & XC3028_FWFLAG_FORMAT)
      {
         m_fw_segs[segindex].format = le16toh(*(reinterpret_cast<uint16_t*> (buf)));
         buf += sizeof(uint16_t);
      }
      if (m_fw_segs[segindex].flags & XC3028_FWFLAG_IFREQ)
      {
         m_fw_segs[segindex].ifreq_hz = le32toh(*(reinterpret_cast<uint32_t*> (buf)));
         buf += sizeof(uint32_t);
      }
      if ((buf + size) > end)
      {
         LIBTUNERERR << "xc3028: size of firmware " << segindex << " extends beyond end of file" << endl;
         error = EINVAL;
         return;
      }
      buf += size;
      ++segindex;  
   }
   if (segindex < m_num_segs)
   {
      LIBTUNERERR << "xc3028: corrupt firmware; found " << segindex << " images, expected " << m_num_segs << endl;
      error = EINVAL;
   }
}

xc3028::~xc3028(void)
{
   delete m_firmware;
   m_firmware = NULL;
   delete[] m_fw_segs;
   m_fw_segs = NULL;
   m_num_segs = 0;
}

int xc3028::load_firmware(const char *filename, bool force)
{
   if (filename == NULL)
   {
      DIAGNOSTIC(LIBTUNERLOG << "XC3028: loadfirmware NULL filename" << endl )
      return EINVAL;
   }
   DIAGNOSTIC(LIBTUNERLOG << "XC3028: load_firmware:" << filename << endl )
   
   int error = 0;
   tuner_firmware fw(filename, error);
   if (error || (!force && fw.up_to_date ()))
   {
      DIAGNOSTIC(LIBTUNERLOG << "XC3028: NOT updating firmware" << endl)
      return error;
   }
   DIAGNOSTIC(LIBTUNERLOG << "XC3028: Updating firmware" << endl)
   uint32_t size_a = le32toh(*((uint32_t*)(fw.buffer())));
   uint32_t size_b = le32toh(*(((uint32_t*)(fw.buffer())) + 1));
   uint8_t *fw_bytes = (uint8_t*)(fw.buffer());
   uint8_t buffer[8];
   int bytes_read;
   if (!error && size_a && (fw.length() > 8))
   {
      error = m_device.write(fw_bytes + 8, size_a);
   }
   if (!error && size_b && (fw.length() > (size_a + 8)))
   {
      /*TODO: Are these sleeps really necessary? They were lifted from the Linux driver...*/
      usleep(1000);
      error = m_device.write(fw_bytes + size_a + 8, size_b);
   }
   if (!error)
   {
      usleep(1000);
      buffer[0] = 0x7F;
      buffer[1] = 0x01;
      error = m_device.write(buffer, 2);
   }
   if (!error)
   {
      usleep(20000);
      error = m_device.write(buffer, 2);
   }
   if (!error)
   {
      usleep(70000);
      buffer[0] = 0x10;
      buffer[1] = 0x10;
      buffer[2] = 0x00;
      error = m_device.write(buffer, 3);
   }
   if (!error)
   {
      usleep(20000);
      buffer[0] = 0x04;
      buffer[1] = 0x17;
      error = m_device.write(buffer, 2);
   }
   if (!error)
   {
      usleep(20000);
      buffer[0] = 0x00;
      buffer[1] = 0x00;
      error = m_device.write(buffer, 2);
   }
   for (bytes_read = 0; (!error && (bytes_read < 8)); bytes_read += 2)
   {
      usleep(20000);
      error = m_device.read(buffer + bytes_read, 2);
   }
   if (!error)
   {
      DIAGNOSTIC(LIBTUNERLOG << "XC3028 Firmware rev. " << setfill('0') << hex <<
         setw(2) << (int)(buffer[1]) << setw(2) << (int)(buffer[0]) << setw(2) << (int)(buffer[3]) << 
         setw(2) << (int)(buffer[2]) << '-' << setw(2) << (int)(buffer[5]) << setw(2) << (int)(buffer[4]) << 
         setw(2) << (int)(buffer[7]) << setw(2) << (int)(buffer[6]) << dec << endl)
      usleep(20000);
      buffer[0] = 0x10;
      buffer[1] = 0x00;
      buffer[2] = 0x00;
      error = m_device.write(buffer, 3);
   }
   if (!error)
   {
      fw.update();  
   }
   return error;
}
/**
 * Analog PAL/NTSC/SECAM tuning?
 */
int xc3028::set_channel(const avb_channel &channel)
{
   int error = pll_driver::set_channel(channel);
   if (!error && (channel.bandwidth_hz == 7000000))
   {
      m_buffer[PLL_BANDSWITCH_BYTE] |= 0x10;
   }
   return error;
}


int xc3028::start(uint32_t timeout_ms)
{
   int error = 0;
   uint8_t buffer[3];
   buffer[0] = 0x04;
   buffer[1] = 0x01;
   switch (m_mode)
   {
      case XC3028_MODE_VSB:
         buffer[2] = 0x50;
         break;
      case XC3028_MODE_QAM64:
      case XC3028_MODE_QAM256:
      case XC3028_MODE_QAM_AUTO:
         buffer[2] = 0x5F;
         break;
      default:
         LIBTUNERERR << "XC3028: Unable to start device: modulation not configured" << endl;
         return ENXIO;
   }
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      LIBTUNERERR << "XC3028: Unable to start device: failed to set operation mode" << endl;
      m_mode = XC3028_MODE_UNKNOWN;
      return error;
   }
   usleep(20000);
   buffer[0] = 0x1C;
   if (m_mode == XC3028_MODE_VSB)
   {
      buffer[1] = 0x03;
   }
   else
   {
      buffer[1] = 0x00;
   }
   buffer[2] = m_mode;
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      LIBTUNERERR << "XC3028: Unable to start device: failed to set receiver/channel mode" << endl;
      m_mode = XC3028_MODE_UNKNOWN;
      return error;
   }
   usleep(30000);
   uint8_t status = 0;
   uint32_t time_slept = 0;
   bool locked = false;
   do
   {
      if ((m_mode = get_mode(status)) == XC3028_MODE_UNKNOWN)
      {
         return ENXIO;
      }
      if (status & 0x01)
      {
         locked = true;
         break;
      }
      usleep(20000);
      time_slept += 50;
   } while (time_slept < timeout_ms);
   if (!locked)
   {
      LIBTUNERERR << "XC3028: demodulator not locked" << endl;
      return ETIMEDOUT;
   }
   else
   {
      unsigned int delay = m_config.get_number<unsigned int>(XC3028_DELAY_KEY);
      if (delay == 0)
      {
         delay = XC3028_DEFAULT_DELAY;
      }
      usleep(delay);
   }
   return 0;
}

uint8_t xc3028::get_mode(uint8_t &status)
{
   static uint8_t buffer[] = {0x04, 0x00};
   uint8_t full_status[2];
   int error = 0;
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      LIBTUNERERR << "XC3028: Failed to request demodulator status" << endl;
      return XC3028_MODE_UNKNOWN;
   }
   usleep(30000);
   if ((error = m_device.read(full_status, sizeof(full_status))))
   {
      LIBTUNERERR << "XC3028: Failed to receive demodulator status" << endl;
      return XC3028_MODE_UNKNOWN;
   }
   status = full_status[1];
   return full_status[0];
}
