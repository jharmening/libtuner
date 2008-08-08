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

#include "dtt7579.h"

dtt7579::dtt7579(tuner_config &config, tuner_device &device)
   :  tuner_driver(config, device),
      pll_driver(config, device, dtt7579_bands, 
         (sizeof(dtt7579_bands) / sizeof(frequency_band)),
         36166667)
{}
      
dtt7579::~dtt7579(void) {}

const frequency_band dtt7579::dtt7579_bands[] =
{
   {177000000, 443250000, 166667, 0xb4, 0x02, PLL_IGNORE_AUX},
   {443250000, 542000000, 166667, 0xb4, 0x08, PLL_IGNORE_AUX},
   {542000000, 771000000, 166667, 0xbc, 0x08, PLL_IGNORE_AUX},
   {771000000, 858000000, 166667, 0xf4, 0x08, PLL_IGNORE_AUX}
};

