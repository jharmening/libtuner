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

      virtual int write(const uint8_t *buffer, size_t size, size_t &written) = 0;

      virtual int read(uint8_t *buffer, size_t size, size_t &read) = 0;

      virtual int write(const uint8_t *buffer, size_t size)
      {
         size_t transferred = 0;
         return write(buffer, size, transferred);
      }

      virtual int read(uint8_t *buffer, size_t size)
      {
         size_t transferred = 0;
         return read(buffer, size, transferred);
      }
      
      virtual int write_array(const uint8_t *buffer, size_t elem_size, size_t total_size);
      
      virtual int read_array(uint8_t *buffer, size_t elem_size, size_t total_size);

      virtual int transact(const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size);
      
   protected:

      tuner_config &m_config;

};


#endif

