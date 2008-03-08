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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/endian.h>
#include <sys/errno.h>
#include <math.h>
#include "tuner_firmware.h"
#include "or51132.h"

#define OR51132_VSB_CONFIG_KEY "OR51132_VSB_FW"
#define OR51132_QAM_CONFIG_KEY "OR51132_QAM_FW"
#define OR51132_DELAY_KEY "OR51132_DELAY"
#define OR51132_DEFAULT_DELAY 500000

#define OR51132_MODE_UNKNOWN  0x00
#define OR51132_MODE_VSB      0x06
#define OR51132_MODE_QAM64    0x43
#define OR51132_MODE_QAM256   0x45
#define OR51132_MODE_QAM_AUTO 0x4F

or51132::or51132(tuner_config &config, tuner_device &device)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     m_config(config)
{
   uint8_t status = 0;
   m_mode = get_mode(status);
}

or51132::~or51132(void)
{
}

int or51132::load_firmware(const char *filename, bool force)
{
   if (filename == NULL)
   {
      return EINVAL;
   }
   int error = 0;
   tuner_firmware fw(filename, error);
   if (error || (!force && fw.up_to_date ()))
   {
      DIAGNOSTIC(LIBTUNERLOG << "OR51132: NOT updating firmware" << endl)
      return error;
   }
   DIAGNOSTIC(LIBTUNERLOG << "OR51132: Updating firmware" << endl)
   uint32_t size_a = le32toh(*((uint32_t*)(fw.buffer())));
   uint32_t size_b = le32toh(*(((uint32_t*)(fw.buffer())) + 1));
   uint8_t *fw_bytes = (uint8_t*)(fw.buffer());
   uint8_t buffer[8];
   int bytes_read;
   if (!error && size_a && (fw.length() > 8))
   {
      error = m_device.write(fw_bytes + 8, size_a);
   }
   if (!error && size_b && (fw.length() > (size_a + 8)))
   {
      /*TODO: Are these sleeps really necessary? They were lifted from the Linux driver...*/
      usleep(1000);
      error = m_device.write(fw_bytes + size_a + 8, size_b);
   }
   if (!error)
   {
      usleep(1000);
      buffer[0] = 0x7F;
      buffer[1] = 0x01;
      error = m_device.write(buffer, 2);
   }
   if (!error)
   {
      usleep(20000);
      error = m_device.write(buffer, 2);
   }
   if (!error)
   {
      usleep(70000);
      buffer[0] = 0x10;
      buffer[1] = 0x10;
      buffer[2] = 0x00;
      error = m_device.write(buffer, 3);
   }
   if (!error)
   {
      usleep(20000);
      buffer[0] = 0x04;
      buffer[1] = 0x17;
      error = m_device.write(buffer, 2);
   }
   if (!error)
   {
      usleep(20000);
      buffer[0] = 0x00;
      buffer[1] = 0x00;
      error = m_device.write(buffer, 2);
   }
   for (bytes_read = 0; (!error && (bytes_read < 8)); bytes_read += 2)
   {
      usleep(20000);
      error = m_device.read(buffer + bytes_read, 2);
   }
   if (!error)
   {
      DIAGNOSTIC(LIBTUNERLOG << "OR51132 Firmware rev. " << setfill('0') << hex <<
         setw(2) << (int)(buffer[1]) << setw(2) << (int)(buffer[0]) << setw(2) << (int)(buffer[3]) << 
         setw(2) << (int)(buffer[2]) << '-' << setw(2) << (int)(buffer[5]) << setw(2) << (int)(buffer[4]) << 
         setw(2) << (int)(buffer[7]) << setw(2) << (int)(buffer[6]) << dec << endl)
      usleep(20000);
      buffer[0] = 0x10;
      buffer[1] = 0x00;
      buffer[2] = 0x00;
      error = m_device.write(buffer, 3);
   }
   if (!error)
   {
      fw.update();  
   }
   return error;
}

int or51132::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = 0;
   interface.bit_endianness = DVB_IFC_BIT_BE;
   interface.polarity = DVB_IFC_NEG_POL;
   interface.input_width_bits = DVB_INPUT_PARALLEL;
   uint8_t old_mode = m_mode;
   switch(channel.modulation)
   {
      case DVB_MOD_VSB_8:
         m_mode = OR51132_MODE_VSB;
         break;
      case DVB_MOD_QAM_64:
         m_mode = OR51132_MODE_QAM64;
         break;
      case DVB_MOD_QAM_256:
         m_mode = OR51132_MODE_QAM256;
         break;
      case DVB_MOD_QAM_AUTO:
         m_mode = OR51132_MODE_QAM_AUTO;
         break;
      default:
         return EINVAL;
   }
   if (m_mode == OR51132_MODE_VSB)
   {
      const char *vsb_fw = m_config.get_string(OR51132_VSB_CONFIG_KEY);
      if (vsb_fw == NULL)
      {
         LIBTUNERERR << "VSB firmware file not configured" << endl;
         return ENOENT;
      }
      interface.clock = DVB_IFC_NORM_CLCK;
      error = load_firmware(vsb_fw, (old_mode != OR51132_MODE_VSB));
   }
   else
   {
      const char *qam_fw = m_config.get_string(OR51132_QAM_CONFIG_KEY);
      if (qam_fw == NULL)
      {
         LIBTUNERERR << "QAM firmware file not configured" << endl;
         return ENOENT;  
      }
      interface.clock = DVB_IFC_PUNC_CLCK;
      error = load_firmware(qam_fw, 
         ((old_mode != OR51132_MODE_QAM64) && (old_mode != OR51132_MODE_QAM256) && (old_mode != OR51132_MODE_QAM_AUTO)));
   }
   if (error)
   {
      m_mode = OR51132_MODE_UNKNOWN;
   }
   return error;
}

int or51132::get_signal(dvb_signal &signal)
{
   int error = 0;
   static uint8_t buffer[] = {0x04, 0x02};
   uint8_t status[2];
   uint32_t noise;
   signal.ber = 0;
   signal.uncorrected_blocks = 0;   
   status[0] = get_mode(status[1]);
   if (status[0] == OR51132_MODE_UNKNOWN)
   {
      LIBTUNERERR << "OR51132: Unable to retrieve signal status: Modulation not set" << endl;
      return ENXIO;
   }
   if (!(status[1] & 0x01))
   {
      signal.locked = false;
      return 0;
   }
   signal.locked = true;
   uint8_t ntsc_correction = 0;
   double snr_const = 0.0;
   switch (status[0])
   {
      case OR51132_MODE_VSB:
         DIAGNOSTIC(LIBTUNERLOG << "OR51132: getting VSB signal" << endl)
         if (status[1] & 0x10)
         {
            ntsc_correction = 3;
         }
      case OR51132_MODE_QAM64:
         DIAGNOSTIC(LIBTUNERLOG << "OR51132: getting QAM64 signal" << endl)
         snr_const = 897152044.8282;
         break;
      case OR51132_MODE_QAM256:
         DIAGNOSTIC(LIBTUNERLOG << "OR51132: getting QAM256 signal" << endl)
         snr_const = 907832426.314266;
         break;
      default:
         LIBTUNERERR << "OR51132: Unrecognized modulation status" << endl;
         return ENXIO;
   }
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      LIBTUNERERR << "OR51132: Unable to request noise value" << endl;
      return error;
   }
   usleep(30000);
   if ((error = m_device.read(status, sizeof(status))))
   {
      LIBTUNERERR << "OR51132: Unable to receive noise value" << endl;
      return error;
   }
   noise = (status[1] << 8) | status[0];
   signal.snr = (10.0 * log10(snr_const / (noise * noise))) - ntsc_correction;
   signal.strength = (signal.snr / 35) * 100;
   return error;
}

int or51132::start(uint32_t timeout_ms)
{
   int error = 0;
   uint8_t buffer[3];
   buffer[0] = 0x04;
   buffer[1] = 0x01;
   switch (m_mode)
   {
      case OR51132_MODE_VSB:
         buffer[2] = 0x50;
         break;
      case OR51132_MODE_QAM64:
      case OR51132_MODE_QAM256:
      case OR51132_MODE_QAM_AUTO:
         buffer[2] = 0x5F;
         break;
      default:
         LIBTUNERERR << "OR51132: Unable to start device: modulation not configured" << endl;
         return ENXIO;
   }
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      LIBTUNERERR << "OR51132: Unable to start device: failed to set operation mode" << endl;
      m_mode = OR51132_MODE_UNKNOWN;
      return error;
   }
   usleep(20000);
   buffer[0] = 0x1C;
   if (m_mode == OR51132_MODE_VSB)
   {
      buffer[1] = 0x03;
   }
   else
   {
      buffer[1] = 0x00;
   }
   buffer[2] = m_mode;
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      LIBTUNERERR << "OR51132: Unable to start device: failed to set receiver/channel mode" << endl;
      m_mode = OR51132_MODE_UNKNOWN;
      return error;
   }
   usleep(30000);
   uint8_t status = 0;
   uint32_t time_slept = 0;
   bool locked = false;
   do
   {
      if ((m_mode = get_mode(status)) == OR51132_MODE_UNKNOWN)
      {
         return ENXIO;
      }
      if (status & 0x01)
      {
         locked = true;
         break;
      }
      usleep(20000);
      time_slept += 50;
   } while (time_slept < timeout_ms);
   if (!locked)
   {
      LIBTUNERERR << "OR51132: demodulator not locked" << endl;
      return ETIMEDOUT;
   }
   else
   {
      unsigned int delay = m_config.get_number<unsigned int>(OR51132_DELAY_KEY);
      if (delay == 0)
      {
         delay = OR51132_DEFAULT_DELAY;
      }
      usleep(delay);
   }
   return 0;
}
      
uint8_t or51132::get_mode(uint8_t &status)
{
   static uint8_t buffer[] = {0x04, 0x00};
   uint8_t full_status[2];
   int error = 0;
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      LIBTUNERERR << "OR51132: Failed to request demodulator status" << endl;
      return OR51132_MODE_UNKNOWN;
   }
   usleep(30000);
   if ((error = m_device.read(full_status, sizeof(full_status))))
   {
      LIBTUNERERR << "OR51132: Failed to receive demodulator status" << endl;
      return OR51132_MODE_UNKNOWN;
   }
   status = full_status[1];
   return full_status[0];
}
