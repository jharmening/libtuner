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

#include "dtt75105.h"

dtt75105::dtt75105(tuner_config &config, tuner_device &device)
   : tuner_driver(config, device),
     pll_driver(config, device, 36166667,
         dtt75105_bands, (sizeof(dtt75105_bands) / sizeof(frequency_band)))
{}

dtt75105::~dtt75105(void) {}

const frequency_band dtt75105::dtt75105_bands[] =
{
   {177000000, 264000000, 166667, 0xb4, 0x02, PLL_IGNORE_AUX},
   {264000000, 470000000, 166667, 0xbc, 0x02, PLL_IGNORE_AUX},
   {470000000, 735000000, 166667, 0xbc, 0x08, PLL_IGNORE_AUX},
   {735000000, 835000000, 166667, 0xf4, 0x08, PLL_IGNORE_AUX},
   {835000000, 896000000, 166667, 0xfc, 0x08, PLL_IGNORE_AUX}
};

int dtt75105::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = pll_driver::set_channel(channel, interface);
   if (!error && (channel.bandwidth_hz == 7000000))
   {
      m_buffer[PLL_BANDSWITCH_BYTE] |= 0x10;
   }
   return error;
}

int dtt75105::set_channel(const avb_channel &channel)
{
   int error = pll_driver::set_channel(channel);
   if (!error && (channel.bandwidth_hz == 7000000))
   {
      m_buffer[PLL_BANDSWITCH_BYTE] |= 0x10;
   }
   return error;
}

