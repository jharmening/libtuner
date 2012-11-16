/*-
 * Copyright 2008 Fritz Katz
 * Copyright 2012 Jason Harmening
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

#include "dvb_driver.h"
#include "avb_driver.h"
#include "tuner_firmware.h"

class xc3028
   : public dvb_driver,
     public avb_driver
{
   public:

#pragma pack(push, 1)

      struct fw_section_header
      {
         #define XC3028_FW_TYPE_BASE   0
         #define XC3028_FW_TYPE_DVB    1
         #define XC3028_FW_TYPE_AVB    2
         #define XC3028_FW_TYPE_SCODE  3
         #define XC3028_FW_TYPE_MAIN   4
         uint16_t type;
         uint16_t num_firmwares;
      };

      struct common_fw_header
      {
         uint32_t offset;
         uint32_t size;
      };

      struct base_fw_header
      {
         common_fw_header common;
         #define XC3028_BASEFW_8MHZ    (1 << 0)
         #define XC3028_BASEFW_FM      (1 << 1)
         #define XC3028_BASEFW_INPUT1  (1 << 2)
         #define XC3028_BASEFW_MTS     (1 << 3)
         uint16_t flags;
      };

      struct dvb_fw_header
      {
         common_fw_header common;
         uint16_t modulation_mask;
         #define XC3028_DVBFW_6MHZ     (1 << 0)
         #define XC3028_DVBFW_7MHZ     (1 << 1)
         #define XC3028_DVBFW_8MHZ     (1 << 2)
         #define XC3028_DVBFW_78MHZ    (1 << 3) // 7MHz VHF + 8MHz UHF
         #define XC3028_DVBFW_D2620    (1 << 4)
         #define XC3028_DVBFW_D2633    (1 << 5)
         uint16_t flags;
      };

      struct avb_fw_header
      {
         common_fw_header common;
         uint32_t video_fmt_mask;
         uint32_t audio_fmt_mask;
         #define XC3028_AVBFW_MTS      (1 << 0)
         #define XC3028_AVBFW_LCD      (1 << 1)
         #define XC3028_AVBFW_NOGD     (1 << 2)
         uint16_t flags;
      };

      struct scode_fw_header
      {
         common_fw_header common;
         uint16_t ifreq_khz;
         #define XC3028_SCFW_MONO      (1 << 0)
         #define XC3028_SCFW_ATSC      (1 << 1)
         #define XC3028_SCFW_IF        (1 << 2)
         #define XC3028_SCFW_LG60      (1 << 3)
         #define XC3028_SCFW_ATI638    (1 << 4)
         #define XC3028_SCFW_OREN538   (1 << 5)
         #define XC3028_SCFW_OREN36    (1 << 6)
         #define XC3028_SCFW_TOYOTA388 (1 << 7)
         #define XC3028_SCFW_TOYOTA794 (1 << 8)
         #define XC3028_SCFW_DIBCOM52  (1 << 9)
         #define XC3028_SCFW_ZARLINK456 (1 << 10)
         #define XC3028_SCFW_CHINA     (1 << 11)
         #define XC3028_SCFW_F6MHZ     (1 << 12)
         #define XC3028_SCFW_INPUT2    (1 << 13)
         uint16_t flags;
      };

#pragma pack(pop)

      enum xc3028_reset_t
      {
         XC3028_TUNER_RESET,
         XC3028_CLOCK_RESET
      };

      typedef int (*xc3028_reset_callback_t) (xc3028_reset_t, void*);
      
      xc3028(
         tuner_config &config,
         tuner_device &device,
         xc3028_reset_callback_t callback,
         void *callback_context,
         int &error);

      virtual ~xc3028(void);

      virtual int set_channel(const avb_channel &channel);
            
      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);

      virtual int start(uint32_t timeout_ms);

      virtual void stop(void);

      virtual void reset(void);

      uint16_t get_firmware_version(void) { return m_firmware_ver; }

      void set_firmware_flags(
         uint16_t base_fw_flags,
         uint16_t dvb_fw_flags,
         uint16_t avb_fw_flags,
         uint16_t scode_fw_flags,
         uint16_t scode_ifreq_khz,
         uint8_t scode_index);

   protected:

      xc3028_reset_callback_t m_callback;
      void *m_callback_context;
      tuner_firmware *m_firmware;
      base_fw_header *m_base_fws;
      uint16_t m_num_base_fws;
      dvb_fw_header *m_dvb_fws;
      uint16_t m_num_dvb_fws;
      avb_fw_header *m_avb_fws;
      uint16_t m_num_avb_fws;
      scode_fw_header *m_scode_fws;
      uint16_t m_num_scode_fws;
      size_t m_main_fw_offset;

      base_fw_header *m_current_base;
      dvb_fw_header *m_current_dvb;
      avb_fw_header *m_current_avb;
      scode_fw_header *m_current_scode;

      uint16_t m_firmware_ver;
      uint16_t m_base_flags;
      uint16_t m_dvb_flags;
      uint16_t m_avb_flags;
      uint16_t m_scode_flags;
      uint16_t m_scode_ifreq_khz;
      uint8_t m_scode_index;

      int load_base_fw(uint16_t flags);
      int load_dvb_fw(uint16_t flags, dvb_modulation_t modulation);
      int load_avb_fw(uint16_t flags, avb_video_fmt_t video_fmt, avb_audio_fmt_t audio_fmt);
      int load_scode_fw(uint16_t flags, uint16_t ifreq_khz);
      int send_firmware(common_fw_header &headeri, const char *fwtypename, uint16_t fwtypeindex);
      int set_frequency(uint64_t frequency_hz);
      bool is_locked(void);
};

#endif
