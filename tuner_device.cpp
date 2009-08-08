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

#include <sys/errno.h>
#include "tuner_device.h"

int tuner_device::write_array(const uint8_t *buffer, size_t elem_size, size_t total_size)
{
   int error = 0;
   if ((total_size % elem_size) != 0)
   {
      return EINVAL;
   }
   for (size_t i = 0; i < total_size; i += elem_size)
   {
      if ((error = write(buffer + i, elem_size)))
      {
         return error;
      }
   }
   return error;
}

int tuner_device::read_array(uint8_t *buffer, size_t elem_size, size_t total_size)
{
   int error = 0;
   if ((total_size % elem_size) != 0)
   {
      return EINVAL;
   }
   for (size_t i = 0; i < total_size; i += elem_size)
   {
      if ((error = read(buffer + i, elem_size)))
      {
         return error;
      }
   }
   return error;
}

int tuner_device::transact(const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size)
{
   int error = write(write_buffer, write_size);
   if (error)
   {
      return error;
   }
   return read(read_buffer, read_size);
}
