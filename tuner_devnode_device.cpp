
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "tuner_devnode_device.h"

tuner_devnode_device::tuner_devnode_device(tuner_config &config, const char *devnode, int &error)
   : tuner_device(config),
     m_devnode_fd(-1)
{
   if (error)
   {
      return;
   }
   if ((m_devnode_fd = open(devnode, O_RDWR)) < 0)
   {
      error = ENOENT;
   }
}

tuner_devnode_device::~tuner_devnode_device(void)
{
   if (m_devnode_fd >= 0)
   {
      close(m_devnode_fd);
   }
}

int tuner_devnode_device::write(uint8_t *buffer, size_t size, size_t &written)
{
   ssize_t retval = ::write(m_devnode_fd, buffer, size);
   if (retval == (ssize_t)-1)
   {
      return errno;
   }
   written = retval;
   return 0;
}

int tuner_devnode_device::read(uint8_t *buffer, size_t size, size_t &read)
{
   ssize_t retval = ::read(m_devnode_fd, buffer, size);
   if (retval == ssize_t(-1))
   {
      return errno;
   }
   read = retval;
   return 0;
}
