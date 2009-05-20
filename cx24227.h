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
 
#ifndef __CX24227_H__
#define __CX24227_H__

#include "dvb_driver.h"
 
class cx24227
   : public dvb_driver
{
   
   public:

      enum cx24227_gpio_t
      {
         CX24227_GPIO_DISABLE = 0,
         CX24227_GPIO_ENABLE
      };
      
      enum cx24227_qam_if_t
      {
         CX24227_QAM_IFREQ_4MHZ = 4000000,
         CX24227_QAM_IFREQ_44MHZ = 44000000
      };
      
      enum cx24227_clock_t
      {
         CX24227_CLK_CONT_INV = 0,
         CX24227_CLK_CONT_NINV,
         CX24227_CLK_NCONT_INV,
         CX24227_CLK_NCONT_NINV
      };

      cx24227(
         tuner_config &config, 
         tuner_device &device,
         dvb_input_t input,
         cx24227_qam_if_t qam_ifreq_hz,
         cx24227_gpio_t gpio,
         cx24227_clock_t clock,
         int &error);
      
      virtual ~cx24227(void);
   
      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);

      virtual int get_signal(dvb_signal &signal);
            
      virtual int start(uint32_t timeout_ms);

      virtual void stop(void) {}

      virtual void reset(void);
      
   private:
   
      dvb_input_t      m_input;
      dvb_inversion_t  m_inversion;
      dvb_modulation_t m_modulation;
      cx24227_qam_if_t m_qam_ifreq;
      
      int set_inversion(void);
      
      int set_ifreq(void);
      
      int qam_optimize(void);
      
      bool is_locked(void);

};

#endif
