/*-
 * xc3028.h 
 *  Copyright 2008 Fritz Katz
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

#ifndef __XC3028_H__
#define __XC3028_H__

#include "pll_driver.h"

#define XC3028_FWFLAG_BASE          (1 << 0)
#define XC3028_FWFLAG_BASE_8MHZ_BW  (1 << 1)
#define XC3028_FWFLAG_MTS           (1 << 2)
#define XC3028_FWFLAG_FM            (1 << 3)
#define XC3028_FWFLAG_FM_INPUT1     (1 << 4)
#define XC3028_FWFLAG_D2620         (1 << 5)
#define XC3028_FWFLAG_D2633         (1 << 6)
#define XC3028_FWFLAG_LCD_DISPLAY   (1 << 7)
#define XC3028_FWFLAG_NO_GROUP_DEL  (1 << 8)
#define XC3028_FWFLAG_6MHZ_DTV      (1 << 9)
#define XC3028_FWFLAG_7MHZ_DTV      (1 << 10)
#define XC3028_FWFLAG_7MHZ_VHF_DTV  (1 << 11)
#define XC3028_FWFLAG_8MHZ_DTV      (1 << 12)
#define XC3028_FWFLAG_VSB           (1 << 13)
#define XC3028_FWFLAG_QAM           (1 << 14)
#define XC3028_FWFLAG_MONO          (1 << 15)
#define XC3028_FWFLAG_VIDEO_FORMAT  (1 << 16)
#define XC3028_FWFLAG_AUDIO_FORMAT  (1 << 17)
#define XC3028_FWFLAG_IFREQ         (1 << 18)

class xc3028
   : public dvb_driver,
     public avb_driver
{
   public:
      
      typedef struct
      {
         uint32_t flags;
         uint16_t size;
         uint32_t video_format;
         uint32_t audio_format;
         uint32_t ifreq_hz;
         void *data;
      } xc3028_fw_header;
            
      typedef int (*xc3028_reset_callback_t) (void*);
      
      xc3028(
         tuner_config &config,
         tuner_device &device,
         xc3028_reset_callback_t callback,
         void *callback_context,
         int &error,
         uint32_t firmware_flags = 0,
         uint32_t ifreq_hz = 0);

      virtual ~xc3028(void);

      virtual int set_channel(const avb_channel &channel);
            
      virtual int set_channel(const dvb_channel &channel);

      virtual int start(uint32_t timeout_ms);

      virtual void stop(void) {}

      virtual void reset(void) {}

   private:

      xc3028_reset_callback_t m_callback;
      void *m_callback_context;
      tuner_firmware *m_firmware;
      xc3028_fw_header *m_fw_segs;
      xc3028_fw_header *m_current_fw;
      uint32_t m_frequency_hz;
      uint32_t m_ifreq_hz;
      uint32_t m_default_flags;
      uint32_t m_flags;
      uint32_t m_video_format;
      uint32_t m_audio_format;
      uint16_t m_num_segs;
      uint16_t m_num_base_images;

};

#endif
