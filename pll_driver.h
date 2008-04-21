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
#include "avb_driver.h"

#define PLL_IGNORE_AUX 0xFF

typedef struct
{
   uint32_t min_frequency;
   uint32_t max_frequency;
   uint32_t step_frequency;
   uint8_t control_byte;
   uint8_t bandswitch_byte;
   uint8_t aux_byte;
} frequency_band;

class pll_driver
   : public dvb_driver,
     public avb_driver
{
   public:

      pll_driver(
         tuner_config &config,
         tuner_device &device,
         const frequency_band *bands,
         size_t num_bands);
      
      virtual ~pll_driver(void)
      {
         stop();
      }

      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);
      
      virtual int set_channel(const avb_channel &channel);
            
      virtual int start(uint32_t timeout_ms);

      virtual void stop(void);

      virtual void reset(void);
      
   protected:

      enum
      {
         PLL_DIV0_BYTE = 0,
         PLL_DIV1_BYTE,
         PLL_CONTROL_BYTE,
         PLL_BANDSWITCH_BYTE,
         PLL_AUX_BYTE,
         PLL_BUF_SIZE
      };
      
      enum
      {
         PLL_UNCONFIGURED,
         PLL_CONFIGURED,
         PLL_LOCKED
      } m_state;

      uint8_t m_buffer[PLL_BUF_SIZE];
      const frequency_band *m_bands;
      size_t m_num_bands;
      
      int set_frequency(uint32_t frequency_hz, uint32_t ifreq_hz);

};

#endif
