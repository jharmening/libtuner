#ifndef __TUNER_DEVNODE_DEVICE_H__
#define __TUNER_DEVNODE_DEVICE_H__

#include "tuner_device.h"

class tuner_devnode_device
   : public tuner_device
{
   public:

      tuner_devnode_device(tuner_config &config, const char *devnode, int &error);

      virtual ~tuner_devnode_device(void);

      virtual int write(uint8_t *buffer, size_t size, size_t &written);

      virtual int read(uint8_t *buffer, size_t size, size_t &read);

   protected:

      int m_devnode_fd;

};

#endif

