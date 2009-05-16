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

#ifndef __MT2131_H__
#define __MT2131_H__

#include "dvb_driver.h"
#include "avb_driver.h"

class mt2131
   : public dvb_driver,
     public avb_driver
{
   public:
   
      mt2131(tuner_config &config, tuner_device &device, int &error);
      
      virtual ~mt2131(void) {}
      
      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);

      virtual int get_signal(dvb_signal &signal)
      {
         return 0;
      }
      
      virtual int set_channel(const avb_channel &channel);
      
      virtual int start(uint32_t timeout_ms);

      virtual void stop(void) {}

      virtual void reset(void) {}
      
   private:
   
      int set_frequency(uint32_t freq_hz);
};

#endif
