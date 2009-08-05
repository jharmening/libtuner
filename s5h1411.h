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
 
#ifndef __S5H1411_H__
#define __S5H1411_H__

#include "dvb_driver.h"

enum s5h1411_gpio_t
{
   S5H1411_GPIO_DISABLE = 0,
   S5H1411_GPIO_ENABLE
};

enum s5h1411_if_t
{
   S5H1411_IFREQ_3_25MHZ = 3250000,
   S5H1411_IFREQ_3_5MHZ = 3500000,
   S5H1411_IFREQ_4MHZ = 4000000,
   S5H1411_IFREQ_5_38MHZ = 5380000,
   S5H1411_IFREQ_44MHZ = 44000000
};

enum s5h1411_clock_t
{
   S5H1411_CLK_CONT_INV = 0,
   S5H1411_CLK_CONT_NINV,
   S5H1411_CLK_NCONT_INV,
   S5H1411_CLK_NCONT_NINV
};

class s5h1411
   : public dvb_driver
{
   
   public:

      s5h1411(
         tuner_config &config, 
         tuner_device &device,
         tuner_device &qam_device,
         dvb_input_t input,
         s5h1411_if_t vsb_ifreq_hz,
         s5h1411_if_t qam_ifreq_hz,
         s5h1411_gpio_t gpio,
         s5h1411_clock_t clock,
         int &error);
      
      virtual ~s5h1411(void);
   
      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);

      virtual int get_signal(dvb_signal &signal);
            
      virtual int start(uint32_t timeout_ms);

      virtual void stop(void);

      virtual void reset(void);
      
   private:
   
      tuner_device    &m_qam_device;
      dvb_input_t      m_input;
      dvb_inversion_t  m_inversion;
      dvb_modulation_t m_modulation;
      s5h1411_if_t     m_vsb_ifreq;
      s5h1411_if_t     m_qam_ifreq;
      
      int set_inversion(void);
      
      int set_ifreq(s5h1411_if_t ifreq_hz);
      
      int qam_optimize(void);
      
      int soft_reset(void);
      
      int i2c_gate(uint8_t value);
      
      bool is_locked(void);

};

#endif
