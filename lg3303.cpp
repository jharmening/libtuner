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
   dvb_polarity_t clock_polarity, dvb_input_t m_input, int &error)
   : dvb_driver(config, device),
     m_modulation(DVB_MOD_UNKNOWN),
     m_clock_polarity(clock_polarity),
     m_input(input)
{
   static const uint8_t init_data[] = {0x4c, 0x14, 0x87, 0xf3};
   if (error)
   {
      return;
   }
   if (clock_polarity == DVB_IFC_POS_POL)
   {
      error = m_device.write(init_data, 4);
   }
   else
   {
      error = m_device.write(init_data, 2);
   }
   if (!error)
   {
      error = reset();  
   }
}

int lg3303::reset(void)
{
   uint8_t buffer[] = {REG_IRQ_STATUS, 0x00}
   int error = m_device.write(buffer, 2);
   if (!error)
   {
      buffer[1] = 0x01;
      error = m_device.write(buffer, 2);  
   }
   return error;
}
      
int lg3303::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = 0;
   interface.bit_endianness = DVB_IFC_BIT_LE;
   interface.polarity = m_clock_polarity;
   interface.input_width_bits = m_input;
   interface.clock = DVB_IFC_NORM_CLCK;
   static const uint8_t vsb_data[] = 
   {
      0x04, 0x00,
      0x0d, 0x40,
      0x0e, 0x87,
      0x0f, 0x8e,
      0x10, 0x01,
      0x47, 0x8b
   };
   
   static const uint8_t qam_data[] =
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

   u8 top_ctrl[] = {REG_TOP_CONTROL, 0x00};
   if (m_input == DVB_INPUT_SERIAL)
   {
      top_ctrl[1] = 0x40;  
   }
   if (m_modulation != channel.modulation)
   {
      switch (channel.modulation)
      {
         case DVB_MOD_VSB_8:
            top_ctrl[1] |= 0x03;
            if ((error = m_device.write(vsb_data, sizeof(vsb_data))))
            {
               return error;
            }
            break;
         case DVB_MOD_QAM_256:
            top_ctrl[1] |= 0x01;
         case DVB_MOD_QAM_64:
            if ((error = m_device.write(qam_data, sizeof(qam_data))))
            {
               return error;
            }
            break;
         default:
            LIBTUNERERR << "LG3303: Unsupported modulation type\n" << endl;
            return EINVAL;
      }
   }
   if ((error = m_device.write(top_ctrl, sizeof(top_ctrl))))
   {
      return error;  
   }
   m_modulation = channel.modulation;
   return reset();
}

