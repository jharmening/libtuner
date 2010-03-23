/*-
 * Copyright 2010 Jason Harmening
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
 #include "tda8295.h"
 
tda8295::tda8295(tuner_config &config, tuner_device &device, int &error)
   : tuner_driver(config, device),
     avb_driver(config, device)
{
   if (error)
   {
      return;
   }
   static const uint8_t mode[] = 
   {
      0x30, 0x01,
      0x00, 0x01,
      0x01, 0x00,
      0x01, 0x01
   };
   error = m_device.write_array(mode, 2, sizeof(mode));
   if (error)
   {
      return;
   }
   usleep(20000);
   static const uint8_t init[] =
   {
      0x01, 0x00,
      0x33, 0x14,
      0x34, 0x00,
      0x3E, 0x63,
      0x38, 0x23,
      0x3F, 0x01,
      0x42, 0x61,
      0x44, 0x0B
   };
   error = m_device.write_array(init, 2, sizeof(init));
   agc_enable(false, error);
   i2c_gate_open(error);
}
      
tda8295::~tda8295(void)
{
   int error = 0;
   do_reset(error);
}

void tda8295::agc_enable(bool enable, int &error)
{
   if (error)
   {
      return;
   }
   uint8_t agc[2];
   agc[0] = 0x02;
   error = m_device.transact(agc, 1, agc + 1, 1);
   if (error)
   {
      return;
   }
   if (enable)
   {
      agc[1] &= 0xBF;
   }
   else
   {
      agc[1] | 0x40;
   }
   error = m_device.write(agc, sizeof(agc));
}

void tda8295::i2c_gate_open(int &error)
{
   if (error)
   {
      return;
   }
   static const uint8_t i2c_gate[] = {0x45, 0xC1};
   error = m_device.write(i2c_gate, sizeof(i2c_gate));
   usleep(20000);
}

void tda8295::i2c_gate_close(int &error)
{
   if (error)
   {
      return;
   }
   uint8_t i2c_gate[3];
   i2c_gate[0] = 0x46;
   error = m_device.transact(i2c_gate, 1, i2c_gate + 1, 1);
   if (!error)
   {
      i2c_gate[2] = i2c_gate[1] & 0xFB;
      i2c_gate[1] = 0x01;
      i2c_gate[0] = 0x45;
      error = m_device.write(i2c_gate, sizeof(i2c_gate));
   }
   if (!error)
   {
      usleep(5000);
      i2c_gate[0] = 0x46;
      i2c_gate[1] = i2c_gate[2] | 0x04;
      error = m_device.write(i2c_gate, 2);
   }
}

void tda8295::do_reset(int &error)
{
   if (error)
   {
      return;
   }
   agc_enable(false, error);
   i2c_gate_open(error);
   if (!error)
   {
      static const uint8_t power[] = {0x30, 0x03};
      error = m_device.write(power, sizeof(power));
   }
}

void tda8295::reset(void)
{
   int error = 0;
   do_reset(error);
}

void tda8295::stop(void)
{
   int error = 0;
   i2c_gate_open(error);
}

int tda8295::set_channel(const avb_channel &channel)
{
   int error = 0;
   static const uint8_t power[] = {0x30, 0x01};
   error = m_device.write(power, sizeof(power));
   agc_enable(true, error);
   if (error)
   {
      return error;
   }
   uint8_t std[3];
   std[0] = 0x00; // STD register address
   std[2] = 0x01; // set easy mode in register 0x01
   switch (channel.video_format)
   {
      case AVB_VIDEO_FMT_NTSC_M:
      case AVB_VIDEO_FMT_NTSC_N:
      case AVB_VIDEO_FMT_NTSC_J:
      case AVB_VIDEO_FMT_PAL_N:
      case AVB_VIDEO_FMT_PAL_NC:
      case AVB_VIDEO_FMT_PAL_M:
         std[1] = 0x01;
         break;
      case AVB_VIDEO_FMT_PAL_B:
      case AVB_VIDEO_FMT_SECAM_B:
         std[1] = 0x02;
         break;
      case AVB_VIDEO_FMT_PAL_G:
      case AVB_VIDEO_FMT_PAL_H:
      case AVB_VIDEO_FMT_SECAM_G:
      case AVB_VIDEO_FMT_SECAM_H:
         std[1] = 0x04;
         break;
      case AVB_VIDEO_FMT_PAL_I:
         std[1] = 0x08;
         break;
      case AVB_VIDEO_FMT_PAL_D:
      case AVB_VIDEO_FMT_PAL_D1:
      case AVB_VIDEO_FMT_SECAM_D:
      case AVB_VIDEO_FMT_SECAM_K:
      case AVB_VIDEO_FMT_SECAM_K1:
         std[1] = 0x10;
         break;
      case AVB_VIDEO_FMT_SECAM_L:
         std[1] = 0x20;
         break;
      case AVB_VIDEO_FMT_SECAM_LC:
         std[1] = 0x40;
         break;
      case AVB_VIDEO_FMT_NONE:
         switch (channel.audio_format)
         {
            case AVB_AUDIO_FMT_FM_MONO:
            case AVB_AUDIO_FMT_FM_MONO_NON_USA:
            case AVB_AUDIO_FMT_FM_MONO_USA:
            case AVB_AUDIO_FMT_FM_STEREO:
            case AVB_AUDIO_FMT_FM_STEREO_NON_USA:
            case AVB_AUDIO_FMT_FM_STEREO_USA:
               std[1] = 0x80;
               break;
            default:
               return EINVAL;
         }
         break;
      default:
         return EINVAL;
   }
   error = m_device.write(std, sizeof(std));
   if (!error)
   {
      usleep(20000);
      std[0] = 0x01;
      std[1] = 0x00;
      error = m_device.write(std, 2);
   }
   if (!error)
   {
      static const uint8_t blanking[] = {0x1D, 0x03};
      error = m_device.write(blanking, sizeof(blanking));
      usleep(20000);
   }
   return error;
}

int tda8295::start(uint32_t timeout_ms)
{
   return 0;
}
