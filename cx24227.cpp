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
#include "cx24227.h"

#define CX24227_VSB_IFREQ 5380000

cx24227::cx24227(
   tuner_config &config, 
   tuner_device &device,
   dvb_input_t input,
   cx24227_qam_if_t qam_ifreq_hz,
   cx24227_gpio_t gpio,
   cx24227_clock_t clock,
   int &error)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     m_input(input),
     m_inversion(DVB_INVERSION_OFF),
     m_modulation(DVB_MOD_VSB_8),
     m_qam_ifreq(qam_ifreq_hz)
{
   static const uint8_t init_config[] = 
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
      error = m_device.write_array(init_config, 3, sizeof(init_config));
   }
   if (!error)
   {
      uint8_t output_mode[3];
      output_mode[0] = 0xAB;
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
   error = (error ? error : set_ifreq());
   if (!error)
   {
      uint8_t gpio_config[3];
      gpio_config[0] = 0xE3;
      error = m_device.transact(gpio_config, 1, &(gpio_config[1]), 2);
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
      uint8_t timing[3];
      timing[0] = 0xAC;
      error = m_device.transact(timing, 1, &(timing[1]), 2);
      if (!error)
      {
         timing[1] &= 0xCF;
			timing[1] |= (clock << 4);
         error = m_device.write(timing, 3);
      }
   }
   reset();
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

void cx24227::reset(void)
{
   uint8_t reset_msg[3];
   reset_msg[0] = 0xF5;
   reset_msg[1] = 0;
   reset_msg[2] = 0;
   if (m_device.write(reset_msg, sizeof(reset_msg)) == 0)
   {
      reset_msg[2] = 1;
      m_device.write(reset_msg, sizeof(reset_msg));	
   }
}

int cx24227::qam_optimize(void)
{
   uint8_t qam_lock_stat[2];
   static uint8_t eq_stat = 0xF0;
   static uint8_t master_stat = 0xF1;
   int error = m_device.transact(&eq_stat, 1, qam_lock_stat, 2);
   if (error)
   {
      return error;
   }
   uint8_t amhum_config[] = 
   {
      0x96, 0x00, 0x08,
      0x93, 0x33, 0x32,
      0x9E, 0x2C, 0x37
   };
   if (qam_lock_stat[0] & 0x20)
   {
      amhum_config[2] = 0x0C;
      if ((qam_lock_stat[1] >= 0x38) && (qam_lock_stat[1] <= 0x68))
      {
         amhum_config[4] = 0x31;
         amhum_config[5] = 0x30;
         amhum_config[7] = 0x28;
         amhum_config[8] = 0x36;
      }
   }
   m_device.write_array(amhum_config, 3, sizeof(amhum_config));
   error = m_device.transact(&master_stat, 1, qam_lock_stat, 2);
   if (error)
   {
      return error;
   }
   if (qam_lock_stat[0] & 0x80)
   {
      uint8_t interleave_config[] = {0x96, 0x00, 0x20};
      m_device.write(interleave_config, sizeof(interleave_config));
      interleave_config[0] = 0xB2;
      m_device.transact(interleave_config, 1, &(interleave_config[1]), 2);
      uint8_t temp = interleave_config[1] >> 4;
      interleave_config[0] = 0xAD;
      m_device.transact(interleave_config, 1, &(interleave_config[1]), 2);
      interleave_config[1] = (interleave_config[1] & 0xF0) | temp;
      m_device.write(interleave_config, sizeof(interleave_config));
      interleave_config[0] = 0xAB;
      m_device.transact(interleave_config, 1, &(interleave_config[1]), 2);
      interleave_config[1] &= 0xEF;
      interleave_config[2] &= 0xFE;
      m_device.write(interleave_config, sizeof(interleave_config));
   }
   else
   {
      uint8_t interleave_config[] = {0x96, 0x00, 0x08};
      m_device.write(interleave_config, sizeof(interleave_config));
      interleave_config[0] = 0xAB;
      m_device.transact(interleave_config, 1, &(interleave_config[1]), 2);
      interleave_config[1] |= 0x10;
      interleave_config[2] &= 0x01;
      m_device.write(interleave_config, sizeof(interleave_config));
   }
   return error;
}

int cx24227::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   reset();
   int error = 0;
   static uint8_t vsb_config[] = {0xF4, 0x00, 0x00};
   static uint8_t qam_config[] = 
   {
      0xF4, 0x00, 0x01,
      0x85, 0x01, 0x10
   };
   switch (channel.modulation)
   {
      case DVB_MOD_VSB_8:
         m_modulation = DVB_MOD_VSB_8;
         if ((m_modulation != DVB_MOD_VSB_8) && (m_qam_ifreq != CX24227_QAM_IFREQ_44MHZ))
         {
            error = set_ifreq();
         }
         if (!error)
         {
            error = m_device.write(vsb_config, sizeof(vsb_config));
         }
         break;
      case DVB_MOD_QAM_64:
      case DVB_MOD_QAM_256:
      case DVB_MOD_QAM_AUTO:
         m_modulation = channel.modulation;
         if ((m_modulation == DVB_MOD_VSB_8) && (m_qam_ifreq != CX24227_QAM_IFREQ_44MHZ))
         {
            error = set_ifreq();
         }
         if (!error)
         {
            error = m_device.write_array(qam_config, 3, sizeof(qam_config));
         }
         if (!error)
         {
            error = qam_optimize();
         }
         break;
      default:
         return EINVAL;
   }
   interface.input_width_bits = m_input;
   interface.clock = DVB_IFC_PUNC_CLCK;
   interface.polarity = DVB_IFC_NEG_POL;
   interface.bit_endianness = DVB_IFC_BIT_BE;
   return error;
}

bool cx24227::is_locked(void)
{
   uint8_t lock_stat[] = {0x00, 0x00};
   static uint8_t stat_reg = 0xF1;
   m_device.transact(&stat_reg, 1, lock_stat, sizeof(lock_stat));
   if (lock_stat[0] & 0x80)
   {
      return true;
   }
   return false;
}

int cx24227::start(uint32_t timeout_ms)
{
   reset();
   uint32_t elapsed = 0;
   bool locked = false;
   while (!(locked = is_locked()) && (elapsed < timeout_ms))
   {
      usleep(50000);
      elapsed += 50;     
   }
   while (elapsed < timeout_ms);
   if (!locked)
   {
      LIBTUNERERR << "CX24227: demodulator not locked" << endl;
      return ETIMEDOUT;
   }
   return 0;
}

int cx24227::get_signal(dvb_signal &signal)
{
   signal.locked = is_locked();
   static uint8_t reg_ucblocks = 0xB5;
   static uint8_t reg_vsb_stat = 0xF1;
   static uint8_t reg_qam_stat = 0xF0;
   uint8_t ucblocks[2];
   m_device.transact(&reg_ucblocks, 1, ucblocks, 2);
   signal.uncorrected_blocks = ((uint32_t)(ucblocks[0]) << 8) | ucblocks[1];
   uint8_t snr[2];
   switch (m_modulation)
   {
      case DVB_MOD_VSB_8:
         m_device.transact(&reg_vsb_stat, 1, snr, 2);
         signal.strength = ((double)((snr[0] & 0x3) << 8) + snr[1]) / 924;
         break;
      default:
         m_device.transact(&reg_qam_stat, 1, snr, 2);
         signal.strength = (double)(267 - snr[1]) / 255;
         break;
   }
   return 0;
}
