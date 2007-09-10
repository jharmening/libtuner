#ifndef __TUNER_DRIVER_H__
#define __TUNER_DRIVER_H__

#ifdef _DIAGNOSTIC
#define DIAGNOSTIC(stmt) stmt;
#else
#define DIAGNOSTIC(stmt)
#endif

#include <sys/types.h>
#include "tuner_config.h"
#include "tuner_device.h"

class tuner_driver
{
   public:

      tuner_driver(tuner_config &config, tuner_device &device)
         : m_config(config),
           m_device(device)
      {}

      virtual ~tuner_driver(void) {}

      virtual int start(uint32_t timeout_ms) = 0;

      virtual void stop(void) = 0;

      virtual void reset(void) = 0;

   protected:

      tuner_config &m_config;
      tuner_device &m_device;

};

#endif
