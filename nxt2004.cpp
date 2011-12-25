/*-
 * Copyright 2011 Jason Harmening <jason.harmening@gmail.com>
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
#include "tuner_firmware.h"
#include "nxt2004.h"

#define CCITT_DIVISOR 0x1021
#define NXT2004_FW_KEY "NXT2004_FW"

using namespace std;

nxt2004::nxt2004(
   tuner_config &config,
   tuner_device &device,
   int &error)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     m_modulation(DVB_MOD_UNKNOWN)
{
   if (error)
   {
      return;
   }
   uint8_t chipid[2];
   chipid[0] = 0x0;
   error = m_device.transact(chipid, 1, &(chipid[1]), 1);
   if (chipid[1] != 0x05)
   {
      LIBTUNERERR << "nxt2004: unrecognized chip ID " << chipid[1] << endl;
      error = ENXIO;
   }
   if (!error)
   {
      error = init();
   }
}

int nxt2004::init_microcontroller(void)
{
   static const uint8_t init_sequence[] =
   {
      0x36, 0x01, 0x23, 0x45, 0x67,
      0x89, 0xAB, 0xCD, 0xEF, 0xC0
   };
   uint8_t buffer[2];
   buffer[0] = 0x2B;
   buffer[1] = 0x0;
   int error = m_device.write(buffer, sizeof(buffer));
   buffer[0] = 0x34;
   buffer[1] = 0x70;
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   buffer[0] = 0x35;
   buffer[1] = 0x4;
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   error = (error ? error : m_device.write(init_sequence, sizeof(init_sequence)));
   buffer[0] = 0x21;
   buffer[1] = 0x80;
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   uint32_t elapsed = 0;
   while (!error)
   {
      error = m_device.transact(buffer, 1, &buffer[1], 1);
      if (buffer[1] == 0)
      {
         break;
      }
      if (elapsed >= 1000000)
      {
         error = (error ? error : ETIMEDOUT);
      }
      usleep(10000);
      elapsed += 10000;
   }
   return error;
}

int nxt2004::start_microcontroller(void)
{
   uint8_t buffer[2];
   buffer[0] = 0x22;
   buffer[1] = 0x0;
   return m_device.write(buffer, sizeof(buffer));
}

int nxt2004::stop_microcontroller(void)
{
   uint8_t buffer[2];
   buffer[0] = 0x22;
   buffer[1] = 0x80;
   int error = m_device.write(buffer, sizeof(buffer));
   buffer[0] = 0x31;
   uint32_t elapsed = 0;
   while (!error)
   {
      error = m_device.transact(buffer, 1, &buffer[1], 1);
      if (buffer[1] & 0x10)
      {
         break;
      }
      if (elapsed >= 1000000)
      {
         error = (error ? error : ETIMEDOUT);
      }
      usleep(10000);
      elapsed += 10000;
   }
   return error;
}

int nxt2004::write_microcontroller(uint8_t *data, size_t num_bytes)
{
   if (num_bytes == 0)
   {
      return EINVAL;
   }
   uint8_t buffer[2];
   buffer[0] = 0x35;
   buffer[1] = data[0];
   int error = m_device.write(buffer, sizeof(buffer));
   data[0] = 0x36;
   error = (error ? error : m_device.write(data, num_bytes));
   data[0] = buffer[1];
   buffer[0] = 0x34;
   buffer[1] = num_bytes - 1;
   if ((data[0] & 0x80) && (data[0] != 0x4))
   {
      buffer[1] |= 0x50;
   }
   else
   {
      buffer[1] |= 0x30;
   }
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   buffer[0] = 0x21;
   buffer[1] = 0x80;
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   error = (error ? error : m_device.transact(buffer, 1, &(buffer[1]), 1));
   if (buffer[1] != 0)
   {
      error = (error ? error : EINVAL);
   }
   return error;
}

int nxt2004::read_microcontroller(uint8_t *data, size_t num_bytes)
{
   if (num_bytes == 0)
   {
      return EINVAL;
   }
   uint8_t buffer[2];
   buffer[0] = 0x35;
   buffer[1] = data[0];
   int error = m_device.write(buffer, sizeof(buffer));
   buffer[0] = 0x34;
   buffer[1] = num_bytes - 1;
   if ((data[0] & 0x80) && (data[0] != 0x4))
   {
      buffer[1] |= 0x40;
   }
   else
   {
      buffer[1] |= 0x20;
   }
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   buffer[0] = 0x21;
   buffer[1] = 0x80;
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   buffer[0] = 0x36;
   error = (error ? error : m_device.transact(buffer, 1, &(data[1]), num_bytes - 1));
   return error;
}

int nxt2004::soft_reset(void)
{
   uint8_t buffer[2];
   buffer[0] = 0x8;
   int error = read_microcontroller(buffer, 2);
   buffer[1] = 0x10;
   error = (error ? error : write_microcontroller(buffer, 2));
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 2));
   return error;
}

int nxt2004::init(void)
{
   uint8_t buffer[256];
   buffer[0] = 0x1E;
   buffer[1] = 0x0;
   int error = m_device.write(buffer, 2);
   if (error)
   {
      return error;
   }
   const char *fwfile = m_config.get_string(NXT2004_FW_KEY);
   if (fwfile == NULL)
   {
      LIBTUNERERR << "nxt2004: Firmware file not configured" << endl;
      return ENOENT;
   }
   buffer[0] = 0x2B;
   buffer[1] = 0x80;
   error = m_device.write(buffer, 2);
   tuner_firmware fw(m_config, fwfile, error);
   if (error)
   {
      LIBTUNERERR << "nxt2004: Unable to create firmware image: " << strerror(errno) << endl;
      return error;
   }
   uint8_t *fwdata = reinterpret_cast<uint8_t*>(fw.buffer());
   buffer[0] = 0x29;
   buffer[1] = 0x10;
   buffer[2] = 0x00;
   buffer[3] = 0x81;
   error = (error ? error : m_device.write(buffer, 4));
   buffer[0] = 0x2C;
   uint16_t crc = 0;
   size_t offset = 0;
   for (size_t i = 0; i < fw.length(); ++i)
   {
      uint16_t comparand = (uint16_t)(fwdata[i]) << 8;
      for (uint8_t shift = 0; shift < 8; ++shift)
      {
         crc <<= 1;
         if ((crc ^ comparand) & (1 << 15))
         {
            crc ^= CCITT_DIVISOR;
         }
         comparand <<= 1;
      }
      size_t bufindex = i - offset + 1;
      buffer[bufindex] = fwdata[i];
      if ((bufindex == 255) || (i == (fw.length() - 1)))
      {
         error = m_device.write(buffer, bufindex + 1);
         if (error)
         {
            break;
         }
         offset = bufindex + 1;
      }
   }
   buffer[1] = crc >> 8;
   buffer[2] = crc & 0xFF;
   error = (error ? error : m_device.write(buffer, 3));
   error = (error ? error : m_device.transact(buffer, 1, &(buffer[1]), 1));
   buffer[0] = 0x2B;
   buffer[1] = 0x80;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x19;
   buffer[1] = 0x1;
   error = (error ? error : m_device.write(buffer, 2));

   // ???
   error = (error ? error : init_microcontroller());
   error = (error ? error : stop_microcontroller());
   error = (error ? error : stop_microcontroller());
   error = (error ? error : init_microcontroller());
   error = (error ? error : stop_microcontroller());

   buffer[0] = 0x8;
   buffer[1] = 0xFF;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[1] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x57;
   buffer[1] = 0xD7;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x35;
   buffer[1] = 0x7;
   buffer[2] = 0xFE;
   error = (error ? error : m_device.write(buffer, 3));
   buffer[0] = 0x34;
   buffer[1] = 0x12;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x21;
   buffer[1] = 0x80;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0xA;
   buffer[1] = 0x21;
   error = (error ? error : m_device.write(buffer, 2));

   buffer[0] = 0x80;
   buffer[1] = 0x1;
   error = (error ? error : write_microcontroller(buffer, 2));

   buffer[0] = 0xE9;
   buffer[1] = 0x7E;
   buffer[2] = 0x0;
   error = (error ? error : m_device.write(buffer, 3));
   buffer[0] = 0xCC;
   buffer[1] = 0x0;
   error = (error ? error : m_device.write(buffer, 2));

   buffer[0] = 0x80;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 2));
   error = (error ? error : soft_reset());
   buffer[0] = 0x80;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x1;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x81;
   buffer[1] = 0x70;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x82;
   buffer[1] = 0x31;
   buffer[2] = 0x5E;
   buffer[3] = 0x66;
   error = (error ? error : write_microcontroller(buffer, 4));
   buffer[0] = 0x88;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x11;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x80;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x40;
   error = (error ? error : write_microcontroller(buffer, 2));

   buffer[0] = 0x10;
   error = (error ? error : m_device.transact(buffer, 1, &(buffer[1]), 1));
   buffer[1] = 0x10;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0xA;
   error = (error ? error : m_device.transact(buffer, 1, &(buffer[1]), 1));
   buffer[1] = 0x21;
   error = (error ? error : m_device.write(buffer, 2));

   error = (error ? error : init_microcontroller());

   buffer[0] = 0xA;
   buffer[1] = 0x21;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0xE9;
   buffer[1] = 0x7E;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0xEA;
   buffer[1] = 0x0;
   error = (error ? error : m_device.write(buffer, 2));

   buffer[0] = 0x80;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 2));
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 2));

   error = (error ? error : soft_reset());
   buffer[0] = 0x80;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x4;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x81;
   buffer[1] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x82;
   buffer[1] = 0x80;
   buffer[2] = 0x0;
   buffer[3] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 4));
   buffer[0] = 0x88;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x11;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x80;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x44;
   error = (error ? error : write_microcontroller(buffer, 2));

   buffer[0] = 0x10;
   error = (error ? error : m_device.transact(buffer, 1, &(buffer[1]), 1));
   buffer[1] = 0x12;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x13;
   buffer[1] = 0x4;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x16;
   buffer[1] = 0x0;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x14;
   buffer[1] = 0x4;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[1] = 0x0;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x17;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x14;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x17;
   error = (error ? error : m_device.write(buffer, 2));

   return error;
}

int nxt2004::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = stop_microcontroller();
   uint8_t buffer[2];
   buffer[0] = 0x14;
   buffer[1] = 0x4;
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   buffer[0] = 0x17;
   buffer[1] = 0x0;
   error = (error ? error : m_device.write(buffer, sizeof(buffer)));
   switch (channel.modulation)
   {
      case DVB_MOD_VSB_8:
         interface.clock = DVB_IFC_NORM_CLCK;
         break;
      case DVB_MOD_QAM_64:
      case DVB_MOD_QAM_256:
      case DVB_MOD_QAM_AUTO:
         interface.clock = DVB_IFC_PUNC_CLCK;
         break;
      default:
         error = (error ? error : EINVAL);
   }
   interface.input_width_bits = DVB_INPUT_PARALLEL; 
   interface.polarity = DVB_IFC_NEG_POL;
   interface.bit_endianness = DVB_IFC_BIT_BE;
   m_modulation = channel.modulation;
   return error;
}

int nxt2004::start(uint32_t timeout_ms)
{
   uint8_t buffer[4];
   buffer[0] = 0x8;
   int error = read_microcontroller(buffer, 2);
   buffer[1] = 0x8;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[1] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x42;
   switch (m_modulation)
   {
      case DVB_MOD_VSB_8:
         buffer[1] = 0x70;
         break;
      case DVB_MOD_QAM_64:
      case DVB_MOD_QAM_256:
      case DVB_MOD_QAM_AUTO:
         buffer[1] = 0x74;
         break;
      default:
         error = (error ? error : EINVAL);
   }
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x57;
   buffer[1] = 0x7;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x58;
   buffer[1] = 0x10;
   buffer[2] = 0x0;
   error = (error ? error : m_device.write(buffer, 3));
   buffer[0] = 0x5C;
   switch (m_modulation)
   {
      case DVB_MOD_VSB_8:
         buffer[1] = 0x60;
         break;
      case DVB_MOD_QAM_64:
         buffer[1] = 0x68;
         break;
      case DVB_MOD_QAM_256:
      default:
         buffer[1] = 0x64;
         break;
   }
   buffer[2] = 0x0;
   error = (error ? error : m_device.write(buffer, 3));
   return error;
   buffer[0] = 0x43;
   buffer[1] = 0x5;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x46;
   buffer[1] = 0x0;
   buffer[2] = 0x0;
   error = (error ? error : m_device.write(buffer, 3));
   buffer[0] = 0x4B;
   buffer[1] = 0x80;
   buffer[2] = 0x0;
   error = (error ? error : m_device.write(buffer, 3));
   buffer[0] = 0x4D;
   buffer[1] = 0x0;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x55;
   buffer[1] = 0x44;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x41;
   buffer[1] = 0x4;
   error = (error ? error : m_device.write(buffer, 2));

   buffer[0] = 0x80;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x24;
   error = (error ? error : write_microcontroller(buffer, 2));
   error = (error ? error : soft_reset());
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x4;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x81;
   buffer[1] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x82;
   buffer[1] = 0x80;
   buffer[2] = 0x0;
   buffer[3] = 0x0;
   error = (error ? error : write_microcontroller(buffer, 4));
   buffer[0] = 0x88;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x11;
   error = (error ? error : write_microcontroller(buffer, 2));
   buffer[0] = 0x80;
   error = (error ? error : read_microcontroller(buffer, 2));
   buffer[1] = 0x44;
   error = (error ? error : write_microcontroller(buffer, 2));

   buffer[0] = 0x30;
   switch (m_modulation)
   {
      case DVB_MOD_VSB_8:
         buffer[1] = 0x0;
         break;
      case DVB_MOD_QAM_64:
         buffer[1] = 0x2;
         break;
      case DVB_MOD_QAM_256:
      default:
         buffer[1] = 0x3;
         break;
   }
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x41;
   buffer[1] = 0x0;
   error = (error ? error : m_device.write(buffer, 2));
   buffer[0] = 0x49;
   buffer[1] = 0x80;
   buffer[2] = 0x0;
   error = (error ? error : m_device.write(buffer, 3));
   buffer[0] = 0x4B;
   error = (error ? error : m_device.write(buffer, 3));
   buffer[0] = 0x41;
   buffer[1] = 0x4;
   error = (error ? error : m_device.write(buffer, 2));
   error = (error ? error : start_microcontroller());
   error = (error ? error : init_microcontroller());
   buffer[0] = 0x5C;
   buffer[1] = 0xF0;
   buffer[2] = 0x0;
   error = (error ? error : m_device.write(buffer, 2));
   if (!error)
   {
      uint32_t elapsed = 0;
      bool locked = false;
      while (!(locked = is_locked()) && (elapsed < timeout_ms))
      {
         usleep(50000);
         elapsed += 50;
      }
      if (!locked)
      {
         LIBTUNERERR << "nxt2004: demodulator not locked" << endl;
         error = ETIMEDOUT;
      }
   }
   return error;
}

bool nxt2004::is_locked(void)
{
   uint8_t addr = 0x31;
   uint8_t lock = 0;
   m_device.transact(&addr, sizeof(addr), &lock, sizeof(lock));
   return ((lock & 0x20) ? true : false);
}

int nxt2004::get_signal(dvb_signal &signal)
{
   signal.locked = is_locked();
   uint8_t buffer[4];
   buffer[0] = 0xA1;
   buffer[1] = 0x0;
   int error = m_device.write(buffer, 2);
   buffer[0] = 0xA6;
   error = (error ? error : read_microcontroller(buffer, 3));
   uint16_t raw_snr = ((uint16_t)(buffer[1]) << 8) | buffer[2];
   struct
   {
      uint16_t snr_min;
      uint16_t snr_max;
      uint16_t base;
      uint16_t coeff;
   } snr_table[] =
   {
      {0x7F00, 0x7FFF, 24, 6},
      {0x7EC0, 0x7F00, 18, 6},
      {0x7C00, 0x7EC0, 12, 6},
      {0x0,    0x7C00, 0,  12}
   };
   signal.snr = 0.0;
   for (size_t i = 0; i < (sizeof(snr_table) / sizeof(snr_table[0])); ++i)
   {
      if (raw_snr > snr_table[i].snr_min)
      {
         signal.snr = snr_table[i].base + (snr_table[i].coeff * (double)(raw_snr - snr_table[i].snr_min) / (snr_table[i].snr_max - snr_table[i].snr_min));
      }
   }
   buffer[0] = 0xE6;
   error = (error ? error : read_microcontroller(buffer, 4));
   signal.ber = 8 * ((buffer[1] << 8) + buffer[2]);
   signal.uncorrected_blocks = buffer[3];
   signal.strength = (signal.snr / 35) * 100;
   return error;
}

