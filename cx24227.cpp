/*-
 * Copyright 2009 Jason Harmening
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

#include "cx24227.h"

#define CX24227_VSB_IFREQ 5380000

cx24227::cx24227(
   tuner_config &config, 
   tuner_device &device,
   dvb_interface &interface,
   cx24227_qam_if_t qam_ifreq_hz,
   cx24227_gpio_t gpio,
   int &error)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     m_interface(interface),
     m_inversion(DVB_INVERSION_OFF),
     m_modulation(DVB_MOD_VSB_8),
     m_qam_ifreq(qam_ifreq_hz)
{
   static const uint8_t config[] = 
   {
   // Reg      Val HI   Val LO
      0xF2,    0x00,    0x00,
      0xFA,    0x00,    0x00,
      0x00,    0x00,    0x71,
      0x01,    0x32,    0x13,
      0x09,    0x00,    0x25,
      0x1C,    0x00,    0x1D,
      0x1F,    0x00,    0x2D,
      0x20,    0x00,    0x1D,
      0x22,    0x00,    0x22,
      0x23,    0x00,    0x20,
      0x29,    0x11,    0x0F,
      0x2A,    0x10,    0xB4,
      0x2B,    0x10,    0xAE,
      0x2C,    0x00,    0x31,
      0x31,    0x01,    0x0D,
      0x32,    0x01,    0x00,
      0x44,    0x05,    0x10,
      0x54,    0x01,    0x04,
      0x58,    0x22,    0x22,
      0x59,    0x11,    0x62,
      0x5A,    0x32,    0x11,
      0x5D,    0x03,    0x70,
      0x5E,    0x02,    0x96,
      0x61,    0x00,    0x10,
      0x63,    0x4A,    0x00,
      0x65,    0x08,    0x00,
      0x71,    0x00,    0x03,
      0x72,    0x04,    0x70,
      0x81,    0x00,    0x02,
      0x82,    0x06,    0x00,
      0x86,    0x00,    0x02,
      0x8A,    0x2C,    0x38,
      0x8B,    0x2A,    0x37,
      0x92,    0x30,    0x2F,
      0x93,    0x33,    0x32,
      0x96,    0x00,    0x0C,
      0x99,    0x01,    0x01,
      0x9C,    0x2E,    0x37,
      0x9D,    0x2C,    0x37,
      0x9E,    0x2C,    0x37,
      0xAB,    0x01,    0x00,
      0xAC,    0x10,    0x03,
      0xAD,    0x10,    0x3F,
      0xE2,    0x01,    0x00,
      0xE3,    0x10,    0x00,
      0x28,    0x10,    0x10,
      0xB1,    0x00,    0x0E
   };
   if (!error)
   {
      error = m_device.write_array(config, 3, sizeof(config));
   }
   if (!error)
   {
      uint8_t output_mode[3];
      output_mode[0] = 0xAB;
      error = m_device.transact(&output_mode, 1, &(output_mode[1]), 2);
      if (!error)
      {
         if ((interface.input_width_bits == DVB_INPUT_SERIAL) && !(output_mode[1] & 0x01))
         {
            output_mode[1] |= 0x01;
            error = m_device.write(output_mode, 3);
         }
         else if ((interface.input_width_bits != DVB_INPUT_SERIAL) && (output_mode[1] & 0x01))
         {
            output_mode[1] &= 0xFE;
            error = m_device.write(output_mode, 3);
         }
      }
   }
   error = (error ? (error : set_inversion()));
   error = (error ? (error : set_ifreq()));
   if (!error)
   {
      uint8_t gpio_config[3];
      gpio_config[0] = 0xE3;
      error = m_device.transact(&gpio_config, 1, &(gpio_config[1]), 2);
      if (!error)
      {
         if ((gpio == CX24227_GPIO_ENABLE) && ((gpio_config[1] & 0x11) != 0x11))
         {
            gpio_config[1] |= 0x11;
            error = m_device.write(gpio_config, 3);
         }
         else if ((gpio == CX24227_GPIO_DISABLE) && (gpio_config[1] & 0x01))
         {
            gpio_config[1] &= 0xFE;
            error = m_device.write(gpio_config, 3);
         }
      }
   }
   if (!error)
   {
      static const uint8_t i2c_gate[] = {0xF3, 0x00, 0x01};
      error = m_device.write(i2c_gate, sizeof(i2c_gate));
   }
}

int cx24227::set_inversion(void)
{
   uint8_t inversion[3];
   inversion[0] = 0x1B;
   if (m_inversion == DVB_INVERSION_ON)
   {
      inversion[1] = 0x11;
      inversion[2] = 0x01;
   }
   else
   {
      inversion[1] = 0x01;
      inversion[1] = 0x10;
   }
   return m_device.write(inversion, sizeof(inversion));
}

int cx24227::set_ifreq(void)
{
   static const uint8_t ifreq_default[] =
   {
   // Reg      Val HI   Val LO
      0x87,    0x01,    0xBE,
      0x88,    0x04,    0x36,
      0x89,    0x05,    0x4D
   };
   static const uint8_t ifreq_4mhz[] = 
   {
   // Reg      Val HI   Val LO
      0x87,    0x01,    0x4B,
      0x88,    0x0C,    0xB5,
      0x89,    0x03,    0xE2
   };
   if ((m_modulation != DVB_MOD_VSB_8) && (m_qam_ifreq == CX24227_QAM_IFREQ_4MHZ))
   {
      return m_device.write_array(ifreq_4mhz, 3, sizeof(ifreq_4mhz));
   }
   else
   {
      return m_device.write_array(ifreq_default, 3, sizeof(ifreq_default));
   }
}
