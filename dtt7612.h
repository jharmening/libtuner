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
