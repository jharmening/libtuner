#ifndef __TUNER_DEVICE_H__
#define __TUNER_DEVICE_H__

#include <sys/types.h>
#include "tuner_config.h"

class tuner_device
{

   public:

      tuner_device(tuner_config &config)
         : m_config(config)
      {}

      virtual ~tuner_device(void) {}

      virtual int write(uint8_t *buffer, size_t size, size_t &written) = 0;

      virtual int read(uint8_t *buffer, size_t size, size_t &read) = 0;

      virtual int write(uint8_t *buffer, size_t size)
      {
         size_t transferred = 0;
         return write(buffer, size, transferred);
      }

      virtual int read(uint8_t *buffer, size_t size)
      {
         size_t transferred = 0;
         return read(buffer, size, transferred);
      }

   protected:

      tuner_config &m_config;

};


#endif

