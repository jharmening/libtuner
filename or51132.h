#ifndef __OR51132_H__
#define __OR51132_H__

#include "dvb_driver.h"

class or51132
   : public dvb_driver
{
   public:

      or51132(tuner_config &config, tuner_device &device);

      virtual ~or51132(void);

      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);

      virtual int get_signal(dvb_signal &signal);

      virtual int start(uint32_t timeout_ms);

      virtual void stop(void) {}

      virtual void reset(void) {}

   private:

      const char *m_vsb_fw;
      const char *m_qam_fw;
      uint8_t m_mode;

      uint8_t get_mode(uint8_t &status);
      int load_firmware(const char *filename, bool force);

};

#endif
