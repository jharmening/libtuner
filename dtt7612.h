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

#ifndef __DTT7612_H__
#define __DTT7612_H__

#include "pll_driver.h"

#define DTT7612_MIN_FREQUENCY 57000000
#define DTT7612_MAX_FREQUENCY 863000000
#define DTT7612_INTERMEDIATE  44000000
#define DTT7612_STEPSIZE      62500
#define DTT7612_CONTROL_BYTE  0x8E

class dtt7612
   : public pll_driver
{

   public:

      dtt7612(tuner_config &config, tuner_device &device)
         : pll_driver(config, device)
      {}

      virtual ~dtt7612(void) {}

   protected:

      static const frequency_range dtt7612_ranges[3];

      virtual void get_ranges(const frequency_range *&ranges, size_t &num_elements)
      {
         ranges = dtt7612_ranges;
         num_elements = sizeof(dtt7612_ranges) / sizeof(frequency_range);
      }

      virtual uint32_t get_min_frequency(void)
      {
         return DTT7612_MIN_FREQUENCY;
      }

};

const dtt7612::frequency_range dtt7612::dtt7612_ranges[] =
{
   {147000000, DTT7612_INTERMEDIATE, DTT7612_STEPSIZE, DTT7612_CONTROL_BYTE, 0x39},
   {417000000, DTT7612_INTERMEDIATE, DTT7612_STEPSIZE, DTT7612_CONTROL_BYTE, 0x3A},
   {DTT7612_MAX_FREQUENCY, DTT7612_INTERMEDIATE, DTT7612_STEPSIZE, DTT7612_CONTROL_BYTE, 0x3C}
};

#endif
