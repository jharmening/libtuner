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

#include <stdio.h>
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
      LIBTUNERERR << "Unable to open device " << devnode << ": " << strerror(errno) << endl;
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
      LIBTUNERERR << "Unable to write to device: " << strerror(errno) << endl;
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
      LIBTUNERERR << "Unable to read from device: " << strerror(errno) << endl;
      return errno;
   }
   read = retval;
   return 0;
}

