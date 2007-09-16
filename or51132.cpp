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

#define OR51132_MODE_UNKNOWN  0x00
#define OR51132_MODE_VSB      0x06
#define OR51132_MODE_QAM64    0x43
#define OR51132_MODE_QAM256   0x45
#define OR51132_MODE_QAM_AUTO 0x4F

or51132::or51132(tuner_config &config, tuner_device &device)
   : dvb_driver(config, device),
     m_vsb_fw(NULL),
     m_qam_fw(NULL)
{
   m_vsb_fw = config.get_string(OR51132_VSB_CONFIG_KEY);
   m_qam_fw = config.get_string(OR51132_QAM_CONFIG_KEY);
   uint8_t status = 0;
   m_mode = get_mode(status);
}

or51132::~or51132(void)
{
   m_vsb_fw = NULL;
   m_qam_fw = NULL;
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
      DIAGNOSTIC(printf("OR51132: NOT updating firmware\n"))
      return error;
   }
   DIAGNOSTIC(printf("OR51132: Updating firmware\n"))
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
      DIAGNOSTIC(printf("OR51132 Firmware revision %02X%02X%02X%02X-%02X%02X%02X%02X\n",
         buffer[1], buffer[0], buffer[3], buffer[2], buffer[5], buffer[4], buffer[7], buffer[6]))
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
   interface.input_width_bits = 8;
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
      interface.clock = DVB_IFC_NORM_CLCK;
      error = load_firmware(m_vsb_fw, (old_mode != OR51132_MODE_VSB));
   }
   else
   {
      interface.clock = DVB_IFC_PUNC_CLCK;
      error = load_firmware(m_qam_fw, 
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
   signal.ber = 0.0;
   signal.uncorrected_blocks = 0;   
   status[0] = get_mode(status[1]);
   if ((status[0] == OR51132_MODE_UNKNOWN) || !(status[1] & 0x01))
   {
      printf("OR51132: Unable to retrieve signal status: no lock\n");
      return ENXIO;
   }
   uint8_t ntsc_correction = 0;
   double snr_const = 0.0;
   switch (status[0])
   {
      case OR51132_MODE_VSB:
         DIAGNOSTIC(printf("OR51132: getting VSB signal\n"))
         if (status[1] & 0x10)
         {
            ntsc_correction = 3;
         }
      case OR51132_MODE_QAM64:
         DIAGNOSTIC(printf("OR51132: getting QAM64 signal\n"))
         snr_const = 897152044.8282;
         break;
      case OR51132_MODE_QAM256:
         DIAGNOSTIC(printf("OR51132: getting QAM256 signal\n"))
         snr_const = 907832426.314266;
         break;
      default:
         DIAGNOSTIC(printf("OR51132: Unrecognized modulation status\n"))
         return ENXIO;
   }
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      printf("OR51132: Unable to request noise value\n");
      return error;
   }
   usleep(30000);
   if ((error = m_device.read(status, sizeof(status))))
   {
      printf("OR51132: Unable to receive noise value\n");
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
         printf("OR51132: Unable to start device: modulation not configured\n");
         return ENXIO;
   }
   if ((error = m_device.write(buffer, sizeof(buffer))))
   {
      printf("OR51132: Unable to start device: failed to set operation mode\n");
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
      printf("OR51132: Unable to start device: failed to set receiver/channel mode\n");
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
      printf("OR51132: demodulator not locked\n");
      return ETIMEDOUT;
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
      printf("OR51132: Failed to request demodulator status\n");
      return OR51132_MODE_UNKNOWN;
   }
   usleep(30000);
   if ((error = m_device.read(full_status, sizeof(full_status))))
   {
      printf("OR51132: Failed to receive demodulator status\n");
      return OR51132_MODE_UNKNOWN;
   }
   status = full_status[1];
   return full_status[0];
}
