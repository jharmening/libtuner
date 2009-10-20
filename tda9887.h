/*-
 * Copyright 2008 Jason Harmening
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

#ifndef __TDA9887_H__
#define __TDA9887_H__

#include "avb_driver.h"

#define TDA9887_OPTION_RADIO_IF_41_3    (1 << 0)
#define TDA9887_OPTION_RADIO_GAIN_NORM  (1 << 1)
#define TDA9887_OPTION_PORT1_ENABLE     (1 << 2)
#define TDA9887_OPTION_PORT2_ENABLE     (1 << 3)

class tda9887
   : public avb_driver
{
   public:
      
      tda9887(
         tuner_config &config,
         tuner_device &device,
         uint16_t options);
      
      virtual ~tda9887(void) 
      {
         reset();
      }
      
      virtual int set_channel(const avb_channel &channel);

      virtual int start(uint32_t timeout_ms);

      virtual void stop(void) 
      {
         reset();
      }
      
      virtual void reset(void);

   private:
      
      uint8_t m_buffer[4];
      uint16_t m_options;
      
};   

#endif
