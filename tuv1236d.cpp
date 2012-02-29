/*-
 * Copyright 2011 Jason Harmening <jason.harmening@gmail.com>
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

#include "tuv1236d.h"

tuv1236d::tuv1236d(tuner_config &config, tuner_device &device)
   : tuner_driver(config, device),
     pll_driver(config, device, tuv1236d_digital_bands, 
         (sizeof(tuv1236d_digital_bands) / sizeof(frequency_band)),
         44000000),
     m_input(TUNER_INPUT_DEFAULT)
{}

tuv1236d::~tuv1236d(void) {}

const frequency_band tuv1236d::tuv1236d_digital_bands[] =
{
   {54000000,  157250000, 62500, 0xc6, 0x41, PLL_IGNORE_AUX},
   {157250000, 454000000, 62500, 0xc6, 0x42, PLL_IGNORE_AUX},
   {454000000, 895000000, 62500, 0xc6, 0x44, PLL_IGNORE_AUX}
};

const frequency_band tuv1236d::tuv1236d_analog_bands[] =
{
   {54000000,  157250000, 62500, 0xce, 0x01, PLL_IGNORE_AUX},
   {157250000, 454000000, 62500, 0xce, 0x02, PLL_IGNORE_AUX},
   {454000000, 895000000, 62500, 0xce, 0x04, PLL_IGNORE_AUX}
};

const frequency_band tuv1236d::tuv1236d_fm_bands[] =
{
   {54000000,  895000000, 50000, 0xc8, 0x01, PLL_IGNORE_AUX}
};

int tuv1236d::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = pll_driver::set_channel(channel, interface);
   if (error)
   {
      return error;
   }
   if (channel.modulation == DVB_MOD_VSB_8)
   {
      m_buffer[PLL_BANDSWITCH_BYTE] &= (~0x08);
   }
   else
   {
      m_buffer[PLL_BANDSWITCH_BYTE] |= 0x08;
   }
   return 0;
}

int tuv1236d::set_channel(const avb_channel &channel)
{
   if (channel.video_format == AVB_VIDEO_FMT_NONE)
   {
      switch (channel.audio_format)
      {
         case AVB_AUDIO_FMT_FM_MONO:
         case AVB_AUDIO_FMT_FM_MONO_NON_USA:
         case AVB_AUDIO_FMT_FM_MONO_USA:
         case AVB_AUDIO_FMT_FM_STEREO:
         case AVB_AUDIO_FMT_FM_STEREO_NON_USA:
         case AVB_AUDIO_FMT_FM_STEREO_USA:
            return set_frequency(channel.frequency_hz, 41300000,
               tuv1236d_fm_bands, sizeof(tuv1236d_fm_bands) / sizeof(frequency_band));
         default:
            break;
      }
   }
   return set_frequency(channel.frequency_hz, 44000000, tuv1236d_analog_bands, 
      sizeof(tuv1236d_analog_bands) / sizeof(frequency_band));
}

void tuv1236d::select_input(tuv1236d::tuner_input input)
{
   m_input = input;
}

int tuv1236d::start(uint32_t timeout_ms)
{
   if (m_input == TUNER_INPUT_1)
   {
      m_buffer[PLL_BANDSWITCH_BYTE] &= (~0x08);
   }
   else if (m_input == TUNER_INPUT_2)
   {
      m_buffer[PLL_BANDSWITCH_BYTE] |= 0x08;
   }
   return pll_driver::start(timeout_ms);
}

