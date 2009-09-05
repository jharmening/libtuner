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
 * XC5000 firmware
 * Copyright (c) 2009, Xceive Corporation <info@xceive.com>
 *
 * Permission to use, copy, modify, and/or distribute this software, only
 * for use with Xceive ICs, for any purpose with or without fee is hereby
 * granted, provided that the above copyright notice and this permission
 * notice appear in all source code copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __XC5000_H__
#define __XC5000_H__

#include "dvb_driver.h"
#include "avb_driver.h"

class xc5000
   : public dvb_driver,
     public avb_driver
{
   public:
   
      typedef int (*xc5000_reset_callback) (xc5000&, void*);
   
      xc5000(
         tuner_config &config, 
         tuner_device &device,
         uint32_t ifreq_hz,
         xc5000_reset_callback reset_cb, 
         void *reset_arg,
         int &error);
   
      virtual ~xc5000(void) {}
      
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
   
      enum xc5000_write_reg_t
      {
         XC5000_REG_INIT            = 0x00,
         XC5000_REG_VIDEO_MODE      = 0x01,
         XC5000_REG_AUDIO_MODE      = 0x02,
         XC5000_REG_INPUT_FREQ      = 0x03,
         XC5000_REG_D_CODE          = 0x04,
         XC5000_REG_OUTPUT_FREQ     = 0x05,
         XC5000_REG_SEEK_MODE       = 0x07,
         XC5000_REG_POWER_DOWN      = 0x0A,
         XC5000_REG_SIGNAL_SOURCE   = 0x0D,
         XC5000_REG_SMOOTHEDCVBS    = 0x0E,
         XC5000_REG_XTAL_FREQ       = 0x0F,
         XC5000_REG_FINE_INPUT_FREQ = 0x10,
         XC5000_REG_DDI_MODE        = 0x11
      };

      enum xc5000_read_reg_t
      {
         XC5000_REG_ADC_ENV         = 0x00,
         XC5000_REG_QUALITY         = 0x01,
         XC5000_REG_FRAME_LINES     = 0x02,
         XC5000_REG_HSYNC_FREQ      = 0x03,
         XC5000_REG_LOCK            = 0x04,
         XC5000_REG_FREQ_ERROR      = 0x05,
         XC5000_REG_SNR             = 0x06,
         XC5000_REG_VERSION         = 0x07,
         XC5000_REG_PRODUCT_ID      = 0x08,
         XC5000_REG_BUSY            = 0x09
      };
      
      enum xc5000_source_t
      {
         XC5000_SOURCE_AIR = 0,
         XC5000_SOURCE_CABLE
      };
      
      int read_reg(xc5000_read_reg_t reg, uint16_t &data);
      
      int write_reg(xc5000_write_reg_t reg, uint16_t data);
   
      int load_firmware(void);
      
      int init(void);

      int set_frequency(uint32_t frequency_hz);
      
      int set_source(xc5000_source_t &source);

      uint32_t m_ifreq_hz;
      bool m_fw_loaded;
      xc5000_reset_callback m_reset_cb;
      void *m_reset_arg;
};

#endif
