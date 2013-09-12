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

#include <sys/errno.h>
#include <unistd.h>
#include "s5h1411.h"

using namespace std;

s5h1411::s5h1411(
   tuner_config &config, 
   tuner_device &device,
   tuner_device &qam_device,
   dvb_input_t input,
   s5h1411_if_t vsb_ifreq_hz,
   s5h1411_if_t qam_ifreq_hz,
   s5h1411_gpio_t gpio,
   s5h1411_clock_t clock,
   int &error)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     m_qam_device(qam_device),
     m_input(input),
     m_inversion(DVB_INVERSION_OFF),
     m_modulation(DVB_MOD_UNKNOWN),
     m_vsb_ifreq(vsb_ifreq_hz),
     m_qam_ifreq(qam_ifreq_hz)
{
   static const uint8_t init_config[] = 
   {
   // Reg      Val HI   Val LO
      0xF4,    0x00,    0x00,
      0xF7,    0x00,    0x00,
      0xF7,    0x00,    0x01,
      0xF3,    0x00,    0x00,
      0x00,    0x00,    0x71,
      0x08,    0x00,    0x47,
      0x1C,    0x04,    0x00,
      0x1E,    0x03,    0x70,
      0x1F,    0x34,    0x2C,
      0x24,    0x02,    0x31,
      0x25,    0x10,    0x11,
      0x26,    0x0F,    0x07,
      0x27,    0x0F,    0x04,
      0x28,    0x07,    0x0F,
      0x29,    0x28,    0x20,
      0x2A,    0x10,    0x2E,
      0x2B,    0x02,    0x20,
      0x2E,    0x0D,    0x0E,
      0x2F,    0x10,    0x13,
      0x31,    0x17,    0x1B,
      0x32,    0x0E,    0x0F,
      0x33,    0x0F,    0x10,
      0x34,    0x17,    0x0E,
      0x35,    0x4B,    0x10,
      0x36,    0x0F,    0x17,
      0x3C,    0x15,    0x77,
      0x3D,    0x08,    0x1A,
      0x3E,    0x77,    0xEE,
      0x40,    0x1E,    0x09,
      0x41,    0x0F,    0x0C,
      0x42,    0x1F,    0x10,
      0x4D,    0x05,    0x09,
      0x4E,    0x0A,    0x00,
      0x50,    0x00,    0x00,
      0x5B,    0x00,    0x00,
      0x5C,    0x00,    0x08,
      0x57,    0x11,    0x01,
      0x65,    0x00,    0x7C,
      0x68,    0x05,    0x12,
      0x69,    0x02,    0x58,
      0x70,    0x00,    0x04,
      0x71,    0x00,    0x07,
      0x76,    0x00,    0xA9,
      0x78,    0x31,    0x41,
      0x7A,    0x31,    0x41,
      0xB3,    0x80,    0x03,
      0xB5,    0xA6,    0xBB,
      0xB6,    0x06,    0x09,
      0xB7,    0x2F,    0x06,
      0xB8,    0x00,    0x3F,
      0xB9,    0x27,    0x00,
      0xBA,    0xFA,    0xC8,
      0xBE,    0x10,    0x03,
      0xBF,    0x10,    0x3F,
      0xCE,    0x20,    0x00,
      0xCF,    0x08,    0x00,
      0xD0,    0x08,    0x00,
      0xD1,    0x04,    0x00,
      0xD2,    0x08,    0x00,
      0xD3,    0x20,    0x00,
      0xD4,    0x30,    0x00,
      0xDB,    0x4A,    0x9B,
      0xDC,    0x10,    0x00,
      0xDE,    0x00,    0x01,
      0xDF,    0x00,    0x00,
      0xE3,    0x03,    0x01,
   };
   if (error)
   {
      return;
   }
   uint8_t chipid[3];
   chipid[0] = 0x5;
   error = m_device.transact(chipid, 1, &(chipid[1]), 2);
   if (!error && ((chipid[1] != 0x0) || (chipid[2] != 0x66)))
   {
      error = ENXIO;
   }
   if (!error)
   {
      error = m_device.write_array(init_config, 3, sizeof(init_config));
   }
   static const uint8_t qam_init_config[] =
   {
   // Reg      Val HI   Val LO
      0xF3,    0x00,    0x00,
      0xF3,    0x00,    0x01,
      0x08,    0x06,    0x00,
      0x18,    0x42,    0x01,
      0x1E,    0x64,    0x76,
      0x21,    0x08,    0x30,
      0x0C,    0x56,    0x79,
      0x0D,    0x57,    0x9B,
      0x24,    0x01,    0x02,
      0x31,    0x74,    0x88,
      0x32,    0x0A,    0x08,
      0x3D,    0x86,    0x89,
      0x49,    0x00,    0x48,
      0x57,    0x20,    0x12,
      0x5D,    0x76,    0x76,
      0x04,    0x04,    0x00,
      0x58,    0x00,    0xC0,
      0x5B,    0x01,    0x00,
   };
   if (!error)
   {
      error = m_qam_device.write_array(qam_init_config, 3, sizeof(qam_init_config));
   }
   if (!error)
   {
      uint8_t output_mode[3];
      output_mode[0] = 0xBD;
      error = m_device.transact(output_mode, 1, &(output_mode[1]), 2);
      if (!error)
      {
         if ((input == DVB_INPUT_SERIAL) && !(output_mode[1] & 0x01))
         {
            output_mode[1] |= 0x01;
            error = m_device.write(output_mode, 3);
         }
         else if ((input != DVB_INPUT_SERIAL) && (output_mode[1] & 0x01))
         {
            output_mode[1] &= 0xFE;
            error = m_device.write(output_mode, 3);
         }
      }
   }
   error = (error ? error : set_inversion());
   error = (error ? error : set_ifreq(vsb_ifreq_hz));
   if (!error)
   {
      uint8_t gpio_config[3];
      gpio_config[0] = 0xE0;
      error = m_device.transact(gpio_config, 1, &(gpio_config[1]), 2);
      if (!error)
      {
         if ((gpio == S5H1411_GPIO_ENABLE) && !(gpio_config[2] & 0x2))
         {
            gpio_config[2] |= 0x2;
            error = m_device.write(gpio_config, 3);
         }
         else if ((gpio == S5H1411_GPIO_DISABLE) && (gpio_config[2] & 0x02))
         {
            gpio_config[2] &= 0xFD;
            error = m_device.write(gpio_config, 3);
         }
      }
   }
   if (!error)
   {
      uint8_t timing[3];
      timing[0] = 0xBE;
      error = m_device.transact(timing, 1, &(timing[1]), 2);
      if (!error)
      {
         timing[1] &= 0xCF;
         timing[1] |= (clock << 4);
         error = m_device.write(timing, 3);
      }
   }
   error = (error ? error : soft_reset());
   error = (error ? error : i2c_gate(0x01));
}

s5h1411::~s5h1411(void)
{
   i2c_gate(0x01);
   static const uint8_t sleep_msg[] =
   {
      0xF4, 0x00, 0x01
   };
   m_device.write(sleep_msg, sizeof(sleep_msg));
}

void s5h1411::reset(void)
{
   i2c_gate(0x01);
}

void s5h1411::stop(void)
{
   i2c_gate(0x01);
}

int s5h1411::i2c_gate(uint8_t value)
{
   uint8_t i2c_gate[] = {0xF5, 0x00, value};
   return m_device.write(i2c_gate, sizeof(i2c_gate));
}

int s5h1411::set_inversion(void)
{
   uint8_t inversion[3];
   inversion[0] = 0x24;
   int error = m_device.transact(inversion, 1, &(inversion[1]), 2);
   if (error)
   {
      return error;
   }
   if (m_inversion == DVB_INVERSION_ON)
   {
      inversion[1] |= 0x10;
   }
   else
   {
      inversion[1] &= 0xEF;
   }
   return m_device.write(inversion, sizeof(inversion));
}

int s5h1411::set_ifreq(s5h1411_if_t ifreq_hz)
{
   static const uint8_t ifreq_default[] =
   {
   // Reg      Val HI   Val LO
      0x38,    0x1B,    0xE4,
      0x39,    0x36,    0x55,
      0x2C,    0x1B,    0xE4
   };
   static const uint8_t ifreq_3_25mhz[] =
   {
   // Reg      Val HI   Val LO
      0x38,    0x10,    0xD5,
      0x39,    0x53,    0x42,
      0x2C,    0x10,    0xD9      
   };
   static const uint8_t ifreq_3_5mhz[] =
   {
   // Reg      Val HI   Val LO
      0x38,    0x12,    0x25,
      0x39,    0x1E,    0x96,
      0x2C,    0x12,    0x25      
   };
   static const uint8_t ifreq_4mhz[] = 
   {
   // Reg      Val HI   Val LO
      0x38,    0x14,    0xBC,
      0x39,    0xB5,    0x3E,
      0x2C,    0x14,    0xBD
   };
   const uint8_t *ifreq_buf = ifreq_default;
   switch (ifreq_hz)
   {
      case S5H1411_IFREQ_3_25MHZ:
         ifreq_buf = ifreq_3_25mhz;
         break;
      case S5H1411_IFREQ_3_5MHZ:
         ifreq_buf = ifreq_3_5mhz;
         break;
      case S5H1411_IFREQ_4MHZ:
         ifreq_buf = ifreq_4mhz;
         break;
      default:
         break;
   }
   int error = m_device.write_array(ifreq_buf, 3, 6);
   if (!error)
   {
      error = m_qam_device.write(ifreq_buf + 6, 3);
   }
   return error;
}

int s5h1411::soft_reset(void)
{
   uint8_t reset_msg[3];
   reset_msg[0] = 0xF7;
   reset_msg[1] = 0;
   reset_msg[2] = 0;
   int error = 0;
   error = m_device.write(reset_msg, sizeof(reset_msg));
   if (!error)
   {
      reset_msg[2] = 1;
      error = m_device.write(reset_msg, sizeof(reset_msg));	
   }
   return error;
}

int s5h1411::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = soft_reset();
   static const uint8_t vsb_config[] =
   {
   // Reg      Val HI   Val LO
      0x00,    0x00,    0x71,
      0xF6,    0x00,    0x00,
      0xCD,    0x00,    0xF1      
   };
   static const uint8_t qam_config1[] =
   {
   // Reg      Val HI   Val LO
      0x00,    0x01,    0x71,
      0xF6,    0x00,    0x01
   };
   static uint8_t qam_config2[] = {0x16, 0x11, 0x01};
   static uint8_t qam_config3[] = {0xCD, 0x00, 0xF0};
   dvb_inversion_t inversion = ((channel.inversion == DVB_INVERSION_AUTO) ? 
      DVB_INVERSION_OFF : channel.inversion);
   if (!error && (inversion != m_inversion))
   {
      m_inversion = inversion;
      error = set_inversion();
   }
   switch (channel.modulation)
   {
      case DVB_MOD_VSB_8:
         if (m_modulation != DVB_MOD_VSB_8)
         {
            m_modulation = DVB_MOD_VSB_8;
            error = set_ifreq(m_vsb_ifreq);
            if (!error)
            {
               error = m_device.write_array(vsb_config, 3, sizeof(vsb_config));
            }
         }
         break;
      case DVB_MOD_QAM_64:
      case DVB_MOD_QAM_256:
      case DVB_MOD_QAM_AUTO:
         if ((m_modulation != DVB_MOD_QAM_64) &&
             (m_modulation != DVB_MOD_QAM_256) &&
             (m_modulation != DVB_MOD_QAM_AUTO))
         {
            m_modulation = channel.modulation;
            error = set_ifreq(m_qam_ifreq);
            if (!error)
            {
               error = m_device.write_array(qam_config1, 3, sizeof(qam_config1));
            }
            if (!error)
            {
               error = m_qam_device.write(qam_config2, sizeof(qam_config2));
            }
            if (!error)
            {
               error = m_device.write(qam_config3, sizeof(qam_config3));
            }
         }
         break;
      default:
         return EINVAL;
   }
   interface.input_width_bits = m_input;
   interface.clock = DVB_IFC_PUNC_CLCK;
   interface.polarity = DVB_IFC_NEG_POL;
   interface.bit_endianness = DVB_IFC_BIT_BE;
   return (error ? error : soft_reset());
}

bool s5h1411::is_locked(void)
{
   uint8_t lock_stat[] = {0x00, 0x00};
   if (m_modulation == DVB_MOD_VSB_8)
   {
      uint8_t stat_reg = 0xF2;
      m_device.transact(&stat_reg, 1, lock_stat, sizeof(lock_stat));
      if (lock_stat[0] & 0x10)
      {
         return true;
      }
   }
   else
   {
      uint8_t stat_reg = 0xF0;
      m_device.transact(&stat_reg, 1, lock_stat, sizeof(lock_stat));
      if (lock_stat[1] & 0x10)
      {
         return true;
      }
   }
   return false;
}

int s5h1411::start(uint32_t timeout_ms)
{
   int error = soft_reset();
   error = error ? error : i2c_gate(0x00);
   if (error)
   {
      return error;
   }
   uint32_t elapsed = 0;
   bool locked = false;
   while (!(locked = is_locked()) && (elapsed < timeout_ms))
   {
      usleep(50000);
      elapsed += 50;     
   }
   if (!locked)
   {
      LIBTUNERERR << "S5H1411: demodulator not locked" << endl;
      return ETIMEDOUT;
   }
   return 0;
}

int s5h1411::get_signal(dvb_signal &signal)
{
   signal.locked = is_locked();
   static uint8_t reg_ucblocks = 0xC9;
   static uint8_t reg_vsb_stat = 0xF2;
   static uint8_t reg_qam_stat = 0xF1;
   uint8_t ucblocks[2];
   m_device.transact(&reg_ucblocks, 1, ucblocks, 2);
   signal.uncorrected_blocks = ((uint32_t)(ucblocks[0]) << 8) | ucblocks[1];
   uint8_t snr[2];
   switch (m_modulation)
   {
      case DVB_MOD_VSB_8:
         m_device.transact(&reg_vsb_stat, 1, snr, 2);
         signal.strength = ((double)((snr[0] & 0x3) << 8) + snr[1]) / 927;
         break;
      default:
         m_device.transact(&reg_qam_stat, 1, snr, 2);
         signal.strength = (double)(67951 - ((snr[0] << 8) | snr[1])) / 65535;
         break;
   }
   signal.strength *= 100;
   if (signal.strength > 100)
   {
      signal.strength = 100;
   }
   signal.snr = 0.0;
   signal.ber = 0;
   return 0;
}
