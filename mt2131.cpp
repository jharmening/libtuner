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

#include <sys/errno.h>
#include "mt2131.h"

#define MT2131_IFREQ1   1220  //kHz
#define MT2131_IFREQ2   44000 //kHz
#define MT2131_FREF     16000 //kHz
#define MT2131_MIN_FREQ 48000000  //Hz
#define MT2131_MAX_FREQ 860000000 //Hz

mt2131::mt2131(tuner_config &config, tuner_device &device, int &error)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     avb_driver(config, device)
{
   static const uint8_t config1[] = 
   {
      0x01, 0x50, 0x00, 0x50, 0x80, 0x00, 0x49, 0xfa, 0x88, 0x08, 0x77,
      0x41, 0x04, 0x00, 0x00, 0x00, 0x32, 0x7f, 0xda, 0x4c, 0x00, 0x10,
      0xaa, 0x78, 0x80, 0xff, 0x68, 0xa0, 0xff, 0xdd, 0x00, 0x00
   };
  
   static const uint8_t config2[] =
   {
      0x0b, 0x09,
      0x15, 0x47,
      0x07, 0xf2,
      0x0b, 0x01
   };
  
   static const uint8_t config3[] = 
   {
      0x10, 0x7f, 0xc8, 0x0a, 0x5f, 0x00, 0x04
   };

   if (!error)
   {
      error = m_device.write(config1, sizeof(config1));
   }
   if (!error)
   {
      error = m_device.write_array(config2, 2, sizeof(config2));
   }
   if (!error)
   {
      error = m_device.write(config3, sizeof(config3));
   }
}

int mt2131::set_frequency(uint32_t freq_hz)
{
   if ((freq_hz > MT2131_MAX_FREQ) || (freq_hz < MT2131_MIN_FREQ))
   {
      return EINVAL;
   }
   uint32_t freq_khz = freq_hz / 1000;
   uint32_t rem = freq_khz % 250;
   freq_khz -= rem;
   uint8_t div_buf[7];
   uint32_t div1 = (((MT2131_IFREQ1 * 1000) + freq_khz) * 64) / (MT2131_FREF / 128);
   uint32_t div2 = (((MT2131_IFREQ1 * 1000) - rem - MT2131_IFREQ2) * 64) / (MT2131_FREF / 128);
   div_buf[0] = 1;
   div_buf[1] = (div1 >> 5) & 0xFF;
   div_buf[2] = div1 & 0x1F;
   div_buf[3] = div1 / 8192;
   div_buf[4] = (div2 >> 5) & 0xFF;
   div_buf[5] = div2 & 0x1F;
   div_buf[6] = div2 / 8192;
   int error = m_device.write(div_buf, sizeof(div_buf));
   if (error)
   {
      return error;
   }
   uint8_t band_center[2];
   band_center[0] = 0xB;
   band_center[1] = (freq_hz - 27500001) / 55000000;
   error = m_device.write(band_center, sizeof(band_center));
   return error;
}

int mt2131::start(uint32_t timeout_ms)
{
   uint32_t time_slept = 0;
   int error = 0;
   static const uint8_t stat_reg = 0x8;
   do
   {
      uint8_t status = 0;
      error = m_device.transact(&stat_reg, 1, &status, 1);
      if (error || ((status & 0x88) == 0x88))
      {
         return error;
      }
   }
   while (time_slept < timeout_ms);
   return error;
}

int mt2131::set_channel(const avb_channel &channel)
{
   return set_frequency(channel.frequency_hz);
}

int mt2131::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   return set_frequency(channel.frequency_hz);
}
