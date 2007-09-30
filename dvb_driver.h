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

#ifndef __DVB_DRIVER_H__
#define __DVB_DRIVER_H__

#include "tuner_driver.h"

enum dvb_modulation_t
{
   DVB_MOD_VSB_8,
   DVB_MOD_VSB_16,
   DVB_MOD_QAM_16,
   DVB_MOD_QAM_32,
   DVB_MOD_QAM_64,
   DVB_MOD_QAM_128,
   DVB_MOD_QAM_256,
   DVB_MOD_QAM_AUTO,
   DVB_MOD_QPSK,
   DVB_MOD_OFDM
};

enum dvb_clock_t
{
   DVB_IFC_NORM_CLCK,
   DVB_IFC_PUNC_CLCK
};

enum dvb_polarity_t
{
   DVB_IFC_NEG_POL,
   DVB_IFC_POS_POL
};

enum dvb_endianness_t
{
   DVB_IFC_BIT_BE,
   DVB_IFC_BIT_LE
};

typedef struct
{
   uint8_t input_width_bits;
   dvb_clock_t clock;
   dvb_polarity_t polarity;
   dvb_endianness_t bit_endianness;
} dvb_interface;

typedef struct
{
   dvb_modulation_t modulation;
   uint32_t frequency_hz;
   uint32_t bandwidth_hz;
} dvb_channel;

typedef struct
{
   double strength;
   double snr;
   double ber;
   uint32_t uncorrected_blocks;
} dvb_signal;

class dvb_driver
   : public tuner_driver
{

   public:

      dvb_driver(tuner_config &config, tuner_device &device)
         : tuner_driver(config, device)
      {}

      virtual ~dvb_driver(void) {}

      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface) = 0;

      virtual int get_signal(dvb_signal &signal)
      {
         return 0;
      }

};

#endif
