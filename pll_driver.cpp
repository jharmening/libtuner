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

pll_driver::pll_driver(
   tuner_config &config,
   tuner_device &device,
   const frequency_band *bands,
   size_t num_bands,
   uint32_t ifreq_hz)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     avb_driver(config, device),
     m_state(PLL_UNCONFIGURED),
     m_bands(bands),
     m_num_bands(num_bands),
     m_ifreq_hz(ifreq_hz)
{
}

int pll_driver::set_frequency(uint32_t frequency_hz, uint32_t ifreq_hz)
{
   return set_frequency(frequency_hz, ifreq_hz, m_bands, m_num_bands);
}

int pll_driver::set_frequency(
   uint32_t frequency_hz,
   uint32_t ifreq_hz,
   const frequency_band *bands,
   size_t num_bands)
{
   size_t i;
   for (i = 0; i < num_bands; ++i)
   {
      if ((bands[i].min_frequency <= frequency_hz) && (bands[i].max_frequency >= frequency_hz))
      {
         uint32_t divider = (ifreq_hz + frequency_hz) / bands[i].step_frequency;
         m_buffer[0] = (uint8_t)(divider >> 8);
         m_buffer[1] = (uint8_t)(divider & 0xFF);
         m_buffer[2] = bands[i].control_byte;
         m_buffer[3] = bands[i].bandswitch_byte;
         m_buffer[4] = bands[i].aux_byte;
         break;
      }
   }
   if (i == num_bands)
   {
      return EINVAL;  
   }
   m_state = PLL_CONFIGURED;
   return 0;
}

int pll_driver::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   return set_frequency(channel.frequency_hz, m_ifreq_hz);
}

int pll_driver::set_channel(const avb_channel &channel)
{
   return set_frequency(channel.frequency_hz, m_ifreq_hz);
}

int pll_driver::start(uint32_t timeout_ms)
{
   if (m_state < PLL_CONFIGURED)
   {
      return ENXIO;
   }
   else if (m_state == PLL_LOCKED)
   {
      return 0;
   }
   int error = 0;
   if (m_buffer[4] != PLL_IGNORE_AUX)
   {
      uint8_t aux_buffer[2];
      aux_buffer[0] = m_buffer[2] | 0x18;
      aux_buffer[1] = m_buffer[4];
      if ((error = m_device.write(aux_buffer, 2)))
      {
         return error;
      }   
   }
   error = m_device.write(m_buffer, 4);
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
         LIBTUNERERR << "PLL timed out waiting for lock" << std::endl;
         error = ETIMEDOUT;
      }
      else
      {
         DIAGNOSTIC(LIBTUNERLOG << "PLL has lock" << std::endl)
         m_state = PLL_LOCKED;
      }
   }
   return error;
}

void pll_driver::do_reset(void)
{
   if (m_state != PLL_UNCONFIGURED)
   {
      uint8_t stop_buffer[2];
      stop_buffer[0] = m_buffer[2] | 0x01;
      stop_buffer[1] = m_buffer[3];
      m_device.write(stop_buffer, 2);
      m_state = PLL_UNCONFIGURED;
   }
}
