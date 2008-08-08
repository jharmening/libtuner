/*-
 * Copyright 2008 Konstantin Dimitrov <kosio.dimitrov@gmail.com>
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

#include "fmd1216me.h"

fmd1216me::fmd1216me(tuner_config &config, tuner_device &device)
   : tuner_driver(config, device),
     pll_driver(config, device, fmd1216me_bands, 
         (sizeof(fmd1216me_bands) / sizeof(frequency_band)),
         36125000)
{}

fmd1216me::~fmd1216me(void) {}

const frequency_band fmd1216me::fmd1216me_bands[] =
{
   {50870000,  143870000, 166667, 0xbc, 0x41, 0xa0},
   {143870000, 158870000, 166667, 0xf4, 0x41, 0xa0},
   {158870000, 329870000, 166667, 0xbc, 0x42, 0xa0},
   {329870000, 441870000, 166667, 0xf4, 0x42, 0xa0},
   {441870000, 625870000, 166667, 0xbc, 0x44, 0xa0},
   {625870000, 803870000, 166667, 0xf4, 0x44, 0xa0},
   {803870000, 858000000, 166667, 0xfc, 0x44, 0xa0}
};

int fmd1216me::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = pll_driver::set_channel(channel, interface);
   if (!error && (channel.bandwidth_hz == 8000000) && (channel.frequency_hz >= 158870000))
   {
      m_buffer[PLL_BANDSWITCH_BYTE] |= 0x08;
   }
   return error;
}

int fmd1216me::set_channel(const avb_channel &channel)
{
   int error = pll_driver::set_channel(channel);
   if (!error && (channel.bandwidth_hz == 8000000) && (channel.frequency_hz >= 158870000))
   {
      m_buffer[PLL_BANDSWITCH_BYTE] |= 0x08;
   }
   return error;
}

