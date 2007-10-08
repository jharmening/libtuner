#include <sys/types.h>
#include <sys/errno.h>
#include <math.h>
#include "lg3303.h"

#define REG_TOP_CONTROL         0x00
#define REG_IRQ_MASK            0x01
#define REG_IRQ_STATUS          0x02
#define REG_VSB_CARRIER_FREQ0   0x16
#define REG_VSB_CARRIER_FREQ1   0x17
#define REG_VSB_CARRIER_FREQ2   0x18
#define REG_VSB_CARRIER_FREQ3   0x19
#define REG_CARRIER_MSEQAM1     0x1a
#define REG_CARRIER_MSEQAM2     0x1b
#define REG_CARRIER_LOCK        0x1c
#define REG_TIMING_RECOVERY     0x1d
#define REG_AGC_DELAY0          0x2a
#define REG_AGC_DELAY1          0x2b
#define REG_AGC_DELAY2          0x2c
#define REG_AGC_RF_BANDWIDTH0   0x2d
#define REG_AGC_RF_BANDWIDTH1   0x2e
#define REG_AGC_RF_BANDWIDTH2   0x2f
#define REG_AGC_LOOP_BANDWIDTH0 0x30
#define REG_AGC_LOOP_BANDWIDTH1 0x31
#define REG_AGC_FUNC_CTRL1      0x32
#define REG_AGC_FUNC_CTRL2      0x33
#define REG_AGC_FUNC_CTRL3      0x34
#define REG_AGC_RFIF_ACC0       0x39
#define REG_AGC_RFIF_ACC1       0x3a
#define REG_AGC_RFIF_ACC2       0x3b
#define REG_AGC_STATUS          0x3f
#define REG_SYNC_STATUS_VSB     0x43
#define REG_DEMUX_CONTROL       0x66
#define REG_EQPH_ERR0           0x6e
#define REG_EQ_ERR1             0x6f
#define REG_EQ_ERR2             0x70
#define REG_PH_ERR1             0x71
#define REG_PH_ERR2             0x72
#define REG_PACKET_ERR_COUNTER1 0x8b
#define REG_PACKET_ERR_COUNTER2 0x8c

lg3303::lg3303(tuner_config &config, tuner_device &device,     
   dvb_polarity_t clock_polarity, dvb_input_t input, int &error)
   : dvb_driver(config, device),
     m_modulation(DVB_MOD_UNKNOWN),
     m_clock_polarity(clock_polarity),
     m_input(input)
{
   static uint8_t init_data[] = {0x4c, 0x14, 0x87, 0xf3};
   if (error)
   {
      return;
   }
   if (clock_polarity == DVB_IFC_POS_POL)
   {
      error = write(init_data, 4);
   }
   else
   {
      error = write(init_data, 2);
   }
   if (!error)
   {
      error = do_reset();  
   }
}

int lg3303::do_reset(void)
{
   uint8_t buffer[] = {REG_IRQ_STATUS, 0x00};
   int error = write(buffer, 2);
   if (!error)
   {
      buffer[1] = 0x01;
      error = write(buffer, 2);  
   }
   return error;
}
      
int lg3303::write(uint8_t *buffer, size_t length)
{
   int error = 0;
   if ((length % 2) != 0)
   {
      return EINVAL;
   }
   for (size_t i = 0; i < length; i += 2)
   {
      if ((error = m_device.write(buffer + i, 2)))
      {
         return error;
      }
   }
   return error;
}

int lg3303::read(uint8_t reg, uint8_t *buffer, size_t length)
{
   int error = m_device.write(&reg, sizeof(reg));
   if (error)
   {
      return error;
   }
   return m_device.read(buffer, length);   
}

int lg3303::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = do_reset();
   if (error)
   {
      return error;
   }
   interface.bit_endianness = DVB_IFC_BIT_LE;
   interface.polarity = m_clock_polarity;
   interface.input_width_bits = m_input;
   interface.clock = DVB_IFC_NORM_CLCK;
   static uint8_t vsb_data[] = 
   {
      0x04, 0x00,
      0x0d, 0x40,
      0x0e, 0x87,
      0x0f, 0x8e,
      0x10, 0x01,
      0x47, 0x8b
   };
   
   static uint8_t qam_data[] =
   {
      0x04, 0x00,
      0x0d, 0x00,
      0x0e, 0x00,
      0x0f, 0x00,
      0x10, 0x00,
      0x51, 0x63,
      0x47, 0x66,
      0x48, 0x66,
      0x4d, 0x1a,
      0x49, 0x08,
      0x4a, 0x9b   
   };

   if (m_modulation != channel.modulation)
   { 
      uint8_t top_ctrl[] = {REG_TOP_CONTROL, 0x00};
      if (m_input == DVB_INPUT_SERIAL)
      {
         top_ctrl[1] = 0x40;  
      }
      switch (channel.modulation)
      {
         case DVB_MOD_VSB_8:
            top_ctrl[1] |= 0x03;
            if ((error = write(vsb_data, sizeof(vsb_data))))
            {
               LIBTUNERERR << "LG3303: Unable to configure 8-VSB modulation" << endl;
               return error;
            }
            break;
         case DVB_MOD_QAM_256:
            top_ctrl[1] |= 0x01;
         case DVB_MOD_QAM_64:
            if ((error = write(qam_data, sizeof(qam_data))))
            {
               LIBTUNERERR << "LG3303: Unable to configure QAM modulation" << endl;
               return error;
            }
            break;
         default:
            LIBTUNERERR << "LG3303: Unsupported modulation type\n" << endl;
            return EINVAL;
      }
      if ((error = write(top_ctrl, sizeof(top_ctrl))))
      {
         return error;  
      }
      m_modulation = channel.modulation;
      error = do_reset();
   }
   return error;
}

int lg3303::check_for_lock(bool &locked)
{
   uint8_t reg, value = 0x00;
   int error = 0;
   locked = false;
   reg = 0x58;
   if ((error = read(reg, &value, sizeof(value))))
   {
      LIBTUNERERR << "LG3303: Unable to retrieve signal status" << endl;
      return error;
   }
   if (!(value & 0x01))
   {
      return 0;  
   }
   reg = REG_CARRIER_LOCK;
   if ((error = read(reg, &value, sizeof(value))))
   {
      return error;
   }
   switch (m_modulation)
   {
      case DVB_MOD_VSB_8:
         if (!(value & 0x80))
         {
            return 0;
         }
         reg = 0x38;
         break;
      case DVB_MOD_QAM_64:
      case DVB_MOD_QAM_256:
         if ((value & 0x07) != 0x07)
         {
            return 0;
         }
         reg = 0x8A;
         break;
      default:
         LIBTUNERERR << "LG3303: Unsupported modulation type" << endl;
         return EINVAL;
   }
   error = read(reg, &value, sizeof(value));
   if (!error && (value & 0x01))
   {
      locked = true;
   }
   return error;
}

int lg3303::start(uint32_t timeout_ms)
{
   uint32_t elapsed = 0;
   bool locked = false;
   do
   {
      check_for_lock(locked);
      usleep(50000);
      elapsed += 50;     
   }
   while (!locked && (elapsed < timeout_ms));
   if (!locked)
   {
      LIBTUNERERR << "LG3303: demodulator not locked" << endl;
      return ETIMEDOUT;
   }
   return 0;
}

int lg3303::get_signal(dvb_signal &signal)
{
   int error = 0;
   uint32_t noise, snr_const;
   uint8_t buffer[5];
   uint8_t reg;
   check_for_lock(signal.locked);
   if (!signal.locked)
   {
      return 0;
   }
   signal.ber = 0.0;
   switch(m_modulation)
   {
      case DVB_MOD_VSB_8:
         reg = REG_EQPH_ERR0;
         if ((error = read(reg, buffer, sizeof(buffer))))
         {
            LIBTUNERERR << "LG3303: Unable to retrieve 8-VSB noise value" << endl;
            return error;
         }
         noise = ((buffer[0] & 7) << 16) | (buffer[3] << 8) | buffer[4];
         snr_const = 25600;
         break;
      case DVB_MOD_QAM_64:
      case DVB_MOD_QAM_256:
         reg = REG_CARRIER_MSEQAM1;
         if ((error = read(reg, buffer, 2)))
         {
            LIBTUNERERR << "LG3303: Unable to retrieve QAM noise value" << endl;
            return error;
         }
         noise = (buffer[0] << 8) | buffer[1];
         if (m_modulation == DVB_MOD_QAM_64)
         {
            snr_const = 688128;
         }
         else
         {
            snr_const = 696320;
         }
         break;
      default:
         LIBTUNERERR << "LG3303: Unsupported modulation type" << endl;
         return EINVAL;
   }
   signal.snr = 10.0 * log10((double)snr_const / noise);
   signal.strength = (signal.snr / 35) * 100;
   reg = REG_PACKET_ERR_COUNTER1;
   if ((error = read(reg, buffer, 2)))
   {
      LIBTUNERERR << "LG3303: Unable to retrieve packet error count" << endl;
      return error;
   }
   signal.uncorrected_blocks = (buffer[0] << 8) | buffer[1];
   return 0;
}
