/*-
 * Copyright 2015 Jason Harmening
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

#include <sys/types.h>
#include <dev/iicbus/iic.h>
#include "tuner_iic_device.h"

tuner_iic_device::tuner_iic_device(tuner_config &config, const char *devnode, uint8_t addr, int &error)
   : tuner_devnode_device(config, devnode, error),
     m_addr(addr << 1)
{
   if (!error) error = ioctl(m_devnode_fd, I2CSADDR, &m_addr);
}

int tuner_iic_device::write_array(const uint8_t *buffer, size_t elem_size, size_t total_size)
{
   int error = 0;
   if ((total_size % elem_size) != 0)
   {
      return EINVAL;
   }

   struct iiccmd cmd;
   cmd.slave = m_addr;
   cmd.count = 0;
   cmd.last = 0;
   cmd.buf = NULL;
   error = ioctl(m_devnode_fd, I2CSTART, &cmd);
   cmd.count = (int)elem_size;
   for (size_t i = 0; !error && i < total_size; i += elem_size)
   {
      cmd.buf = (char*)buffer + i;
      if ((i + elem_size) >= total_size) cmd.last = 1;
      error = ioctl(m_devnode_fd, I2CWRITE, &cmd);
      if (!error && !cmd.last) error = ioctl(m_devnode_fd, I2CRPTSTART, &cmd);
   }
   ioctl(m_devnode_fd, I2CSTOP);
   return error;
}

int tuner_iic_device::read_array(uint8_t *buffer, size_t elem_size, size_t total_size)
{
   int error = 0;
   if ((total_size % elem_size) != 0)
   {
      return EINVAL;
   }

   struct iiccmd cmd;
   cmd.slave = m_addr | 1;
   cmd.count = 0;
   cmd.last = 0;
   cmd.buf = NULL;
   error = ioctl(m_devnode_fd, I2CSTART, &cmd);
   cmd.count = (int)elem_size;
   for (size_t i = 0; !error && i < total_size; i += elem_size)
   {
      cmd.buf = (char*)buffer + i;
      if ((i + elem_size) >= total_size) cmd.last = 1;
      error = ioctl(m_devnode_fd, I2CREAD, &cmd);
      if (!error && !cmd.last) error = ioctl(m_devnode_fd, I2CRPTSTART, &cmd);
   }
   ioctl(m_devnode_fd, I2CSTOP);
   return error;

}

int tuner_iic_device::transact(const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size)
{
   struct iiccmd cmd;
   cmd.slave = m_addr;
   cmd.count = 0;
   cmd.last = 0;
   cmd.buf = NULL;

   int error = ioctl(m_devnode_fd, I2CSTART, &cmd);
   cmd.count = (int)write_size;
   cmd.buf = (char*)write_buffer;
   cmd.last = 1;
   error = (error ? error : ioctl(m_devnode_fd, I2CWRITE, &cmd));
   cmd.slave |= 1;
   error = (error ? error : ioctl(m_devnode_fd, I2CRPTSTART, &cmd));
   cmd.count = (int)read_size;
   cmd.buf = (char*)read_buffer;
   cmd.last = 1;
   error = (error ? error : ioctl(m_devnode_fd, I2CREAD, &cmd));
   ioctl(m_devnode_fd, I2CSTOP);

   return error;
}

