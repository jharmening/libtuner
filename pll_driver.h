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

#ifndef __DVB_PLL_DRIVER_H__
#define __DVB_PLL_DRIVER_H__

#include "dvb_driver.h"

class pll_driver
   : public dvb_driver
{
   public:

      pll_driver(tuner_config &config, tuner_device &device)
         : dvb_driver(config, device),
           m_state(DVB_PLL_UNCONFIGURED)
      {
         m_frequency_hz = 0;
      }

      virtual ~pll_driver(void)
      {
         stop();
      }

      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);

      virtual int start(uint32_t timeout_ms);

      virtual void stop(void);

      virtual void reset(void);

   protected:

      typedef struct
      {
         uint32_t max_frequency;
         uint32_t intermediate_frequency;
         uint32_t step_size;
         uint8_t control_byte;
         uint8_t bandswitch_byte;
      } frequency_range;

      virtual void get_ranges(const frequency_range *&ranges, size_t &num_elements) = 0;

      virtual uint32_t get_min_frequency(void) = 0;

   private:

      enum
      {
         DVB_PLL_UNCONFIGURED,
         DVB_PLL_CONFIGURED,
         DVB_PLL_LOCKED
      } m_state;

      uint8_t m_buffer[4];
      uint32_t m_frequency_hz;

      int set_frequency(uint32_t frequency_hz);
};

#endif
