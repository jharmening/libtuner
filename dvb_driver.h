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
   DVB_MOD_UNKNOWN,
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

enum dvb_inversion_t
{
   DVB_INVERSION_AUTO,
   DVB_INVERSION_OFF,
   DVB_INVERSION_ON
};

enum dvb_fec_t
{
   DVB_FEC_NONE,
   DVB_FEC_1_2,
   DVB_FEC_2_3,
   DVB_FEC_3_4,
   DVB_FEC_4_5,
   DVB_FEC_5_6,
   DVB_FEC_6_7,
   DVB_FEC_7_8,
   DVB_FEC_8_9,
   DVB_FEC_AUTO
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

enum dvb_input_t
{
   DVB_INPUT_SERIAL = 1,
   DVB_INPUT_PARALLEL = 8
};

typedef struct
{
   uint8_t input_width_bits;
   dvb_clock_t clock;
   dvb_polarity_t polarity;
   dvb_endianness_t bit_endianness;
} dvb_interface;

#define DVB_SYMBOL_RATE_AUTO 0

typedef struct
{
   uint32_t symbol_rate;
   dvb_fec_t code_rate;
} dvb_qam_settings;

typedef dvb_qam_settings dvb_qpsk_settings;

enum dvb_subcarrier_mode_t
{
   DVB_OFDM_2K_SUBCARRIERS,
   DVB_OFDM_4K_SUBCARRIERS,
   DVB_OFDM_8K_SUBCARRIERS,
   DVB_OFDM_AUTO_SUBCARRIERS
};

enum dvb_guard_interval_t
{  
   DVB_OFDM_GUARD_1_32,
   DVB_OFDM_GUARD_1_16,
   DVB_OFDM_GUARD_1_8,
   DVB_OFDM_GUARD_1_4,
   DVB_OFDM_GUARD_AUTO 
};

enum dvb_hierarchy_t
{
   DVB_OFDM_HIERARCHY_NONE,
   DVB_OFDM_HIERARCHY_AUTO,
   DVB_OFDM_HIERARCHY_1,
   DVB_OFDM_HIERARCHY_2,
   DVB_OFDM_HIERARCHY_4
};

typedef struct
{         
   dvb_modulation_t subcarrier_modulation;
   dvb_fec_t high_prio_code_rate;
   dvb_fec_t low_prio_code_rate;
   dvb_subcarrier_mode_t subcarrier_mode;
   dvb_guard_interval_t guard_interval;
   dvb_hierarchy_t hierarchy;
} dvb_ofdm_settings;

typedef struct
{
   dvb_modulation_t modulation;
   uint64_t frequency_hz;
   uint32_t bandwidth_hz;
   dvb_inversion_t inversion;
   union
   {
      dvb_qam_settings qam;
      dvb_qpsk_settings qpsk;
      dvb_ofdm_settings ofdm;
   } modulation_settings;
} dvb_channel;

typedef struct
{
   bool locked;
   double strength;
   double snr;
   uint32_t ber;
   uint32_t uncorrected_blocks;
} dvb_signal;

class dvb_driver
   : public virtual tuner_driver
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
