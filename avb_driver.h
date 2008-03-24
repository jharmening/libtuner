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

#ifndef __AVB_DRIVER_H__
#define __AVB_DRIVER_H__

#include "tuner_driver.h"

enum avb_video_format
{
   AVB_VIDEO_FORMAT_NONE,
   AVB_VIDEO_FORMAT_NTSC_M,
   AVB_VIDEO_FORMAT_NTSC_N,
   AVB_VIDEO_FORMAT_NTSC_J,
   AVB_VIDEO_FORMAT_NTSC_443,
   AVB_VIDEO_FORMAT_PAL_B,
   AVB_VIDEO_FORMAT_PAL_D,
   AVB_VIDEO_FORMAT_PAL_G,
   AVB_VIDEO_FORMAT_PAL_H,
   AVB_VIDEO_FORMAT_PAL_I,
   AVB_VIDEO_FORMAT_PAL_K,
   AVB_VIDEO_FORMAT_PAL_N,
   AVB_VIDEO_FORMAT_PAL_NC,
   AVB_VIDEO_FORMAT_PAL_M,
   AVB_VIDEO_FORMAT_PAL_60,
   AVB_VIDEO_FORMAT_SECAM_L,
   AVB_VIDEO_FORMAT_SECAM_LC,
   AVB_VIDEO_FORMAT_SECAM_B,
   AVB_VIDEO_FORMAT_SECAM_D,
   AVB_VIDEO_FORMAT_SECAM_G,
   AVB_VIDEO_FORMAT_SECAM_H,
   AVB_VIDEO_FORMAT_SECAM_K
};

enum avb_audio_format
{
   AVB_AUDIO_FORMAT_NONE,
   AVB_AUDIO_FORMAT_BTSC,
   AVB_AUDIO_FORMAT_EIAJ,
   AVB_AUDIO_FORMAT_A2,
   AVB_AUDIO_FORMAT_NICAM,
   AVB_AUDIO_FORMAT_FM_MONO,
   AVB_AUDIO_FORMAT_FM_STEREO,
   AVB_AUDIO_FORMAT_AM_MONO,
   AVB_AUDIO_FORMAT_AM_STEREO
};

typedef struct
{
   avb_video_format video_format;
   uint64_t frequency_hz;
   uint32_t bandwidth_hz;
   avb_audio_format audio_format;
   uint32_t audio_subcarrier_hz;
} avb_channel;

class avb_driver
   : public virtual tuner_driver
{
   public:
       
      avb_driver(tuner_config &config, tuner_device &device)
         : tuner_driver(config, device)
      {}

      virtual ~avb_driver(void) {}
      
      virtual int set_channel(const avb_channel &channel) = 0;
   
};
   
#endif
