/*-
 * Copyright 2007 Jason Harmening
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
#include "pll_driver.h"

int pll_driver::set_frequency(uint32_t frequency_hz)
{   
   size_t range_size;
   const frequency_range *ranges;
   if ((m_state > DVB_PLL_UNCONFIGURED) && (frequency_hz == m_frequency_hz))
   {
      return 0;
   }
   get_ranges(ranges, range_size);
   if ((frequency_hz < get_min_frequency()) ||
      (frequency_hz > ranges[range_size - 1].max_frequency))
   {
      return EINVAL;
   }
   for (size_t i = 0; i < range_size; ++i)
   {
      if (ranges[i].max_frequency >= frequency_hz)
      {
         uint32_t divider = (ranges[i].intermediate_frequency + frequency_hz) / ranges[i].step_size;
         m_buffer[0] = (uint8_t)(divider >> 8);
         m_buffer[1] = (uint8_t)(divider & 0xFF);
         m_buffer[2] = ranges[i].control_byte;
         m_buffer[3] = ranges[i].bandswitch_byte;
         break;
      }
   }
   m_frequency_hz = frequency_hz;
   m_state = DVB_PLL_CONFIGURED;
   return 0;
}

int pll_driver::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   return set_frequency(channel.frequency_hz);
}

int pll_driver::start(uint32_t timeout_ms)
{
   if (m_state != DVB_PLL_CONFIGURED)
   {
      return ENXIO;
   }
   int error = m_device.write(m_buffer, sizeof(m_buffer));
   if (!error)
   {
      uint32_t time_slept = 0;
      bool locked = false;
      uint8_t status = 0;
      do
      {
         usleep(50000);
         error = m_device.read(&status, sizeof(status));
         if (error)
         {
            break;
         }
         if (status & (1 << 6))
         {
            locked = true;
            break;
         }
         time_slept += 50;
      } while (time_slept < timeout_ms);
      if (!locked)
      {
         LIBTUNERERR << "PLL timed out waiting for lock" << endl;
         error = ETIMEDOUT;
      }
      else
      {
         DIAGNOSTIC(LIBTUNERLOG << "PLL has lock" << endl)
         m_state = DVB_PLL_LOCKED;
      }
   }
   return error;
}

void pll_driver::stop(void)
{
   if (m_state != DVB_PLL_UNCONFIGURED)
   {
      uint8_t old_cb = m_buffer[2];
      m_buffer[2] |= 0x01;
      m_device.write(m_buffer, sizeof(m_buffer));
      m_buffer[2] = old_cb;
      m_state = DVB_PLL_CONFIGURED;
   }
}

void pll_driver::reset(void)
{
   stop();
   m_frequency_hz = 0;
   m_state = DVB_PLL_UNCONFIGURED;
}
