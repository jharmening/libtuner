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

#include "dtt7612.h"

dtt7612::dtt7612(tuner_config &config, tuner_device &device)
   :  tuner_driver(config, device),
      pll_driver(config, device, dtt7612_bands, 
         (sizeof(dtt7612_bands) / sizeof(frequency_band)),
         44000000)
{}
      
dtt7612::~dtt7612(void) {}

const frequency_band dtt7612::dtt7612_bands[] =
{
   {57000000,  147000000, 62500, 0x8E, 0x39, 0x50},
   {147000000, 417000000, 62500, 0x8E, 0x3A, 0x50},
   {417000000, 863000000, 62500, 0x8E, 0x3C, 0x50}
};

const frequency_band dtt7612::dtt7612_fm_bands[] =
{
   {57000000,  147000000, 50000, 0x88, 0x39, 0x50},
   {147000000, 417000000, 50000, 0x88, 0x3A, 0x50},
   {417000000, 863000000, 50000, 0x88, 0x3C, 0x50}
};

int dtt7612::set_channel(const avb_channel &channel)
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
               dtt7612_fm_bands, sizeof(dtt7612_fm_bands) / sizeof(frequency_band));
         default:
            break;
      }
   }
   return pll_driver::set_channel(channel);
}
