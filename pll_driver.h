#ifndef __DVB_PLL_DRIVER_H__
#define __DVB_PLL_DRIVER_H__

#include "dvb_driver.h"

class pll_driver
   : public dvb_driver
{
   public:

      pll_driver(tuner_config &config, tuner_device &device)
         : dvb_driver(config, device),
           m_state(DVB_PLL_UNCONFIGURED)
      {
         m_frequency_hz = 0;
      }

      virtual ~pll_driver(void)
      {
         stop();
      }

      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);

      virtual int start(uint32_t timeout_ms);

      virtual void stop(void);

      virtual void reset(void);

   protected:

      typedef struct
      {
         uint32_t max_frequency;
         uint32_t intermediate_frequency;
         uint32_t step_size;
         uint8_t control_byte;
         uint8_t bandswitch_byte;
      } frequency_range;

      virtual void get_ranges(const frequency_range *&ranges, size_t &num_elements) = 0;

      virtual uint32_t get_min_frequency(void) = 0;

   private:

      enum
      {
         DVB_PLL_UNCONFIGURED,
         DVB_PLL_CONFIGURED,
         DVB_PLL_LOCKED
      } m_state;

      uint8_t m_buffer[4];
      uint32_t m_frequency_hz;

      int set_frequency(uint32_t frequency_hz);
};

#endif
