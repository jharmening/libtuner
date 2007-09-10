#ifndef __DVB_DRIVER_H__
#define __DVB_DRIVER_H__

#include "tuner_driver.h"

enum dvb_modulation_t
{
   DVB_MOD_VSB_8,
   DVB_MOD_VSB_16,
   DVB_MOD_QAM_16,
   DVB_MOD_QAM_32,
   DVB_MOD_QAM_64,
   DVB_MOD_QAM_128,
   DVB_MOD_QAM_256,
   DVB_MOD_QAM_AUTO,
   DVB_MOD_QPSK,
   DVB_MOD_OFDM,
   DVB_MODS
};

#define DVB_MOD_NONE DVB_MODS
#define DVB_MOD_CAPABILITY(modulation) (1 << modulation)

enum dvb_clock_t
{
   DVB_IFC_NORM_CLCK,
   DVB_IFC_PUNC_CLCK
};

enum dvb_polarity_t
{
   DVB_IFC_NEG_POL,
   DVB_IFC_POS_POL
};

enum dvb_endianness_t
{
   DVB_IFC_BIT_LE,
   DVB_IFC_BIT_BE
};

typedef struct
{
   uint8_t input_width_bits;
   dvb_clock_t clock;
   dvb_polarity_t polarity;
   dvb_endianness_t bit_endianness;
} dvb_interface;

typedef struct
{
   dvb_modulation_t modulation;
   uint32_t frequency_hz;
   uint32_t bandwidth_hz;
} dvb_channel;

typedef struct
{
   double strength;
   double snr;
   double ber;
   uint32_t uncorrected_blocks;
} dvb_signal;

class dvb_driver
   : public tuner_driver
{

   public:

      dvb_driver(tuner_config &config, tuner_device &device)
         : tuner_driver(config, device)
      {}

      virtual ~dvb_driver(void) {}

      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface) = 0;

      virtual int get_signal(dvb_signal &signal)
      {
         return 0;
      }

};

#endif
