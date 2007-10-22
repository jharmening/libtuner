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

#ifndef __CX22702_H__
#define __CX22702_H__

#include "dvb_driver.h"

class cx22702
   : public dvb_driver
{
   public:
         
      cx22702(tuner_config &config, tuner_device &device, dvb_input_t input, int &error);
      
      virtual ~cx22702(void) {}
      
      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);
      
      virtual int get_signal(dvb_signal &signal);
      
      virtual int start(uint32_t timeout_ms);
      
      virtual void stop(void) {}
      
      virtual void reset(void) {}
      
      int enable_pll(void);
      
      int disable_pll(void);
      
   private:
      
      int check_for_lock(bool &locked);
      
      uint8_t m_input;
      
      uint8_t m_uncorrected_blocks;
};

#include "dvb_driver.h"

#endif
