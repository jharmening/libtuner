/*-
 * Copyright 2008 Jason Harmening
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
#include "tda9887.h"

#define TDA9887_REG_SWITCHING_MODE 0
#define TDA9887_PORT2_DISABLE      (1 << 7)
#define TDA9887_PORT1_DISABLE      (1 << 6)
#define TDA9887_AUDIO_MUTE         (1 << 5)
#define TDA9887_POS_AM_TV          0
#define TDA9887_NEG_FM_TV          (1 << 4)
#define TDA9887_FM_RADIO           (1 << 3)
#define TDA9887_CARRIER_QSS        (1 << 2)
#define TDA9887_FM_AUTO_MUTE       (1 << 1)
#define TDA9887_SOUND_TRAP_BYPASS  (1 << 0)

#define TDA9887_REG_ADJUST_MODE    1
#define TDA9887_AUDIO_GAIN_6DB     (1 << 7)
#define TDA9887_DEEMPHASIS_75      0
#define TDA9887_DEEMPHASIS_50      (1 << 6)
#define TDA9887_DEEMPHASIS_ON      (1 << 5)
#define TDA9887_TOP_ADJUST(dB)     ((dB + 16) & 0x1F)

#define TDA9887_REG_DATA_MODE      2
#define TDA9887_AGC_EXTERNAL       (1 << 7)
#define TDA9887_AGC_L_STD_GATING   (1 << 6)
#define TDA9887_AGC_LOW_GAIN       (1 << 5)
#define TDA9887_VIF_58_75          0
#define TDA9887_VIF_45_75          (1 << 2)
#define TDA9887_VIF_38_90          (2 << 2)
#define TDA9887_VIF_38_00          (3 << 2)
#define TDA9887_VIF_33_90          (4 << 2)
#define TDA9887_VIF_33_40          (5 << 2)
#define TDA9887_VIF_45_75_EXTFM    (6 << 2)
#define TDA9887_VIF_38_90_EXTFM    (7 << 2)
#define TDA9887_RIF_33_3           0
#define TDA9887_RIF_41_3           (1 << 2)
#define TDA9887_AUDIO_IF_4_5       0
#define TDA9887_AUDIO_IF_5_5       1
#define TDA9887_AUDIO_IF_6_0       2
#define TDA9887_AUDIO_IF_6_5       3

tda9887::tda9887(
   tuner_config &config,
   tuner_device &device,
   tda9887_port_state port1,
   tda9887_port_state port2)
   : tuner_driver(config, device),
     avb_driver(config, device),
     m_port1(port1),
     m_port2(port2)
{
   m_buffer[0] = TDA9887_REG_SWITCHING_MODE;
}

int tda9887::set_channel(const avb_channel &channel)
{
   switch (channel.format)
   {
      case AVB_FORMAT_FM_MONO:
         m_buffer[1] = TDA9887_FM_RADIO | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_DEEMPHASIS_ON | TDA9887_DEEMPHASIS_75 | TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AGC_LOW_GAIN | TDA9887_AUDIO_IF_5_5 | TDA9887_VIF_38_90_EXTFM;
         break;
      case AVB_FORMAT_FM_STEREO:
         m_buffer[1] = TDA9887_FM_RADIO | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_AUDIO_GAIN_6DB | TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AGC_LOW_GAIN | TDA9887_AUDIO_IF_5_5 | TDA9887_VIF_38_90_EXTFM;
         break; 
      case AVB_FORMAT_NTSC_J:
         m_buffer[1] = TDA9887_NEG_FM_TV | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_DEEMPHASIS_ON | TDA9887_DEEMPHASIS_50 | TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AUDIO_IF_4_5 | TDA9887_VIF_58_75 | TDA9887_AGC_L_STD_GATING;
         break;
      case AVB_FORMAT_NTSC_M:
      case AVB_FORMAT_NTSC_N:
      case AVB_FORMAT_NTSC_443:
      case AVB_FORMAT_PAL_M:
      case AVB_FORMAT_PAL_NC:
         m_buffer[1] = TDA9887_NEG_FM_TV | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_DEEMPHASIS_ON | TDA9887_DEEMPHASIS_75 | TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AUDIO_IF_4_5 | TDA9887_VIF_45_75 | TDA9887_AGC_L_STD_GATING;
         break;
      case AVB_FORMAT_PAL_B:
      case AVB_FORMAT_PAL_G:
      case AVB_FORMAT_PAL_H:
      case AVB_FORMAT_PAL_N:
         m_buffer[1] = TDA9887_NEG_FM_TV | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_DEEMPHASIS_ON | TDA9887_DEEMPHASIS_50 | TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AUDIO_IF_5_5 | TDA9887_VIF_38_90 | TDA9887_AGC_L_STD_GATING;
         break;
      case AVB_FORMAT_PAL_I:
         m_buffer[1] = TDA9887_NEG_FM_TV | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_DEEMPHASIS_ON | TDA9887_DEEMPHASIS_50 | TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AUDIO_IF_6_0 | TDA9887_VIF_38_90 | TDA9887_AGC_L_STD_GATING;
         break;
      case AVB_FORMAT_PAL_D:
      case AVB_FORMAT_PAL_D1:
      case AVB_FORMAT_PAL_K:
      case AVB_FORMAT_SECAM_D:
      case AVB_FORMAT_SECAM_K:
      case AVB_FORMAT_SECAM_K1:
         m_buffer[1] = TDA9887_NEG_FM_TV | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_DEEMPHASIS_ON | TDA9887_DEEMPHASIS_50 | TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AUDIO_IF_6_5 | TDA9887_VIF_38_90 | TDA9887_AGC_L_STD_GATING;
         break;
      case AVB_FORMAT_SECAM_B:
      case AVB_FORMAT_SECAM_G:
      case AVB_FORMAT_SECAM_H:
         m_buffer[1] = TDA9887_POS_AM_TV | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AUDIO_IF_5_5 | TDA9887_VIF_38_90 | TDA9887_AGC_L_STD_GATING;
         break;
      case AVB_FORMAT_SECAM_L:
         m_buffer[1] = TDA9887_POS_AM_TV | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AUDIO_IF_6_5 | TDA9887_VIF_38_90 | TDA9887_AGC_L_STD_GATING;
         break;
      case AVB_FORMAT_SECAM_LC:
         m_buffer[1] = TDA9887_PORT2_DISABLE | TDA9887_POS_AM_TV | TDA9887_CARRIER_QSS;
         m_buffer[2] = TDA9887_TOP_ADJUST(0);
         m_buffer[3] = TDA9887_AUDIO_IF_6_5 | TDA9887_VIF_33_90 | TDA9887_AGC_L_STD_GATING;
         break;
      default:
         LIBTUNERERR << "tda9887: Invalid broadcast format: " << channel.format << endl;
         return EINVAL;
   }
   if (m_port1 == TDA9887_PORT_ACTIVE)
   {
      m_buffer[1] &= ~(TDA9887_PORT1_DISABLE);
   }
   else if (m_port1 == TDA9887_PORT_INACTIVE)
   {
      m_buffer[1] |= TDA9887_PORT1_DISABLE;
   }
   if (m_port2 == TDA9887_PORT_ACTIVE)
   {
      m_buffer[1] &= ~(TDA9887_PORT2_DISABLE);
   }
   else if (m_port2 == TDA9887_PORT_INACTIVE)
   {
      m_buffer[1] |= TDA9887_PORT2_DISABLE;
   }
   return 0;
}

int tda9887::start(uint32_t timeout_ms)
{
   return m_device.write(m_buffer, sizeof(m_buffer));
}
