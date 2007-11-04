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
#include "cx22702.h"

cx22702::cx22702(tuner_config &config, tuner_device &device, dvb_input_t input, int &error)
   : dvb_driver(config, device),
     m_input(input),
     m_uncorrected_blocks(0)
{
   static const uint8_t init_data1 [] = {0x00, 0x02};
   static const uint8_t init_data2 [] = 
   {
      0x00, 0x00,
      0x0B, 0x06,
      0x09, 0x01,
      0x0D, 0x41,
      0x16, 0x32,
      0x20, 0x0A,
      0x21, 0x17,
      0x24, 0x3e,
      0x26, 0xff,
      0x27, 0x10,
      0x28, 0x00,
      0x29, 0x00,
      0x2a, 0x10,
      0x2b, 0x00,
      0x2c, 0x10,
      0x2d, 0x00,
      0x48, 0xd4,
      0x49, 0x56,
      0x6b, 0x1e,
      0xc8, 0x02,
      0xf9, 0x00,
      0xfa, 0x00,
      0xfb, 0x00,
      0xfc, 0x00,
      0xfd, 0x00
   };
   uint8_t input_mode [] = {0xF8, 0x00};
   error = m_device.write(init_data1, sizeof(init_data1));
   usleep(10000);
   if (!error)
   {
      error = m_device.write_array(init_data2, 2, sizeof(init_data2));
   }
   if (input == DVB_INPUT_SERIAL)
   {
      input_mode[1] = 0x02;  
   }
   if (!error)
   {
      error = m_device.write(input_mode, 2);  
   }
   if (!error)
   {
      error = enable_pll();
   }
}

int cx22702::enable_pll(void)
{
   uint8_t buffer[] = {0x0D, 0x00};
   int error = m_device.transact(buffer, 1, &(buffer[1]), 1);
   if (!error)
   {
      DIAGNOSTIC(printf("CX22702: enable_pll() read 0x%x\n", buffer[1]))
      buffer[1] &= 0xFE;
      error = m_device.write(buffer, sizeof(buffer));
   }
   return error;
}

int cx22702::disable_pll(void)
{
   uint8_t buffer[] = {0x0D, 0x00};
   int error = m_device.transact(buffer, 1, &(buffer[1]), 1);
   if (!error)
   {
      buffer[1] |= 0x01;
      error = m_device.write(buffer, sizeof(buffer));
   }
   return error;  
}

int cx22702::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   interface.input_width_bits = m_input;
   interface.clock = DVB_IFC_NORM_CLCK;
   interface.polarity = DVB_IFC_NEG_POL;
   interface.bit_endianness = DVB_IFC_BIT_LE;
   uint8_t transaction[] = {0x0C, 0x00};
   int error = m_device.transact(transaction, 1, &(transaction[1]), 1);
   if (error)
   {
      return error;  
   }
   DIAGNOSTIC(printf("CX22702: set_channel() status = 0x%x\n", transaction[1]))
   transaction[1] &= 0xCE;
   if (channel.inversion == DVB_INVERSION_ON)
   {
      transaction[1] |= 0x01;
   }
   switch (channel.bandwidth_hz)
   {
      case 6000000:
         transaction[1] |= 0x20;
         break;
      case 7000000:
         transaction[1] |= 0x10;
         break;
      case 8000000:
         break;
      default:
         LIBTUNERERR << "CX22702: Invalid bandwidth setting: " << channel.bandwidth_hz << endl;
         return EINVAL;
   }
   if ((error = m_device.write(transaction, sizeof(transaction))))
   {
      return error;
   }
   // Auto hierarchy and subcarrier modulation
   transaction[0] = 0x06;
   transaction[1] = 0x10;
   if ((error = m_device.write(transaction, sizeof(transaction))))
   {
      return error;
   }
   // Auto hi/lo priority code rates
   transaction[0] = 0x07;
   transaction[1] = 0x09;
   if ((error = m_device.write(transaction, sizeof(transaction))))
   {
      return error;
   }
   // Auto guard interval and transmission mode
   transaction[0] = 0x08;
   transaction[1] = 0xC1;
   if ((error = m_device.write(transaction, sizeof(transaction))))
   {
      return error;
   }

   transaction[0] = 0x0B;
   if ((error = m_device.transact(transaction, 1, &(transaction[1]), 1)))
   {
      return error;
   }
   transaction[1] &= 0xFC;
   if ((error = m_device.write(transaction, sizeof(transaction))))
   {
      return error;
   }
   transaction[0] = 0x0C;
   if ((error = m_device.transact(transaction, 1, &(transaction[1]), 1)))
   {
      return error;
   }
   transaction[1] = ((transaction[1] & 0xBF) | 0x40);
   if ((error = m_device.write(transaction, sizeof(transaction))))
   {
      return error;
   }
   transaction[0] = 0x00;
   transaction[1] = 0x01;
   if ((error = m_device.write(transaction, sizeof(transaction))))
   {
      return error;
   }
   return error;
}

int cx22702::check_for_lock(bool &locked)
{
   locked = false;
   DIAGNOSTIC(
      ({
         uint8_t reg23[] = {0x23, 0x00};
         m_device.transact(reg23, 1, &(reg23[1]), 1);
         printf("CX22702: read 0x%x from register 0x23\n", reg23[1]);
      }))
   uint8_t status[] = {0x0A, 0x00};
   int error = m_device.transact(status, 1, &(status[1]), 1);
   if (error)
   {
      LIBTUNERERR << "CX22702: Unable to retrieve lock status" << endl;
      return error;
   }
   DIAGNOSTIC(printf("CX22702: check_for_lock() read 0x%x\n", status[1]))
   if (status[1] & 0x10)
   {
      locked = true;  
   }
   return error;
}

int cx22702::start(uint32_t timeout_ms)
{
   uint32_t elapsed = 0;
   bool locked = false;
   int error = 0;
   do
   {
      error = check_for_lock(locked);
      if (error || locked)
      {
         break;
      }
      usleep(50000);
      elapsed += 50;     
   }
   while (elapsed < timeout_ms);
   if (!locked)
   {
      LIBTUNERERR << "CX22702: demodulator not locked" << endl;
      return ETIMEDOUT;
   }
   return error;
}

int cx22702::get_signal(dvb_signal &signal)
{
   int error = check_for_lock(signal.locked);
   if (error || !signal.locked)
   {
      return error;
   }
   uint8_t reg = 0xE4;
   uint8_t value = 0x00;
   uint8_t ber_hi = 0x00;
   uint8_t ber_lo = 0x00;
   uint16_t ber = 0;
   if ((error = m_device.transact(&reg, sizeof(reg), &value, sizeof(value))))
   {
      return error;
   }
   reg = 0xDE;
   if ((error = m_device.transact(&reg, sizeof(reg), &ber_hi, sizeof(value))))
   {
      return error;
   }
   reg = 0xDF;
   if ((error = m_device.transact(&reg, sizeof(reg), &ber_lo, sizeof(value))))
   {
      return error;
   }
   if (value & 0x02)
   {
      ber = ((ber_hi & 0x7F) << 7) | (ber_lo & 0x7F);
   }
   else
   {
      ber = ((ber_hi & 0x7F) << 8) | ber_lo;  
   }
   signal.ber = ber;
   signal.snr = 0.0;
   signal.strength = ((double)(~ber) / 0xFFFF) * 100;
   reg = 0xE3;
   if ((error = m_device.transact(&reg, sizeof(reg), &value, sizeof(value))))
   {
      return error;  
   }
   if (value >= m_uncorrected_blocks)
   {
      signal.uncorrected_blocks = value - m_uncorrected_blocks;  
   }
   else
   {
      signal.uncorrected_blocks = value + 0x100 - m_uncorrected_blocks; 
   }
   m_uncorrected_blocks = value;
   return error;
}
