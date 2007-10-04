#ifndef __LGH064F_H__
#define __LGH064F_H__

#include "pll_driver.h"

class lgh064f
   : public pll_driver
{
   
   public:
      
      lgh064f(tuner_config &config, tuner_device &device)
         : pll_driver(config, device, 44000000,
              lgh064f_bands, (sizeof(lgh064f_bands) / sizeof(frequency_band)))
      {}
      
      virtual ~lgh064f(void) {}
      
   protected:
      
      static const frequency_band lgh064f_bands[3];
   
};

const frequency_band lgh064f::lgh064f_bands[] =
{
   {54000000,  165000000, 62500, 0xCE, 0x01, 0x50},
   {165000000, 450000000, 62500, 0xCE, 0x02, 0x50},
   {450000000, 863000000, 62500, 0xCE, 0x04, 0x50}
};

#endif
