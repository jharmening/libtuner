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

#ifndef __TDA18271_H__
#define __TDA18271_H__

#include "dvb_driver.h"
#include "avb_driver.h"

enum tda18271_mode_t
{
   TDA18271_MODE_SINGLE,
   TDA18271_MODE_MASTER,
   TDA18271_MODE_SLAVE
};

typedef struct
{
   uint32_t ifreq_hz;
   uint32_t std:3;
   uint32_t agc_mode:2;
   uint32_t if_level:3;
   uint32_t fm_rfn:1;
   uint32_t if_notch:1;
   uint32_t rf_agc_top:7;
} tda18271_interface;

enum tda18271_version
{
   TDA18271_VER_1 = 0,
   TDA18271_VER_2
};

class tda18271;

typedef int (*tda18271_analog_ifc_callback)(tda18271 &driver, const avb_channel &channel, tda18271_interface &ifc);
typedef int (*tda18271_digital_ifc_callback)(tda18271 &driver, const dvb_channel &channel, tda18271_interface &ifc);

class tda18271
   : public dvb_driver,
     public avb_driver
{
   public:
   
      tda18271(
         tuner_config &config,
         tuner_device &device, 
         tda18271_mode_t mode,
         tda18271_analog_ifc_callback analog_cb,
         tda18271_digital_ifc_callback digital_cb,
         int &error);
      
      virtual ~tda18271(void);
      
      tda18271_version get_version(void);
      
      virtual int set_channel(const avb_channel &channel);
      
      virtual int set_channel(const dvb_channel &channel, dvb_interface &interface);

      virtual int get_signal(dvb_signal &signal);
      
      virtual int start(uint32_t timeout_ms);

      virtual void stop(void);

      virtual void reset(void);
   
   private:
   
      enum tda18271_reg_t
      {
         TDA18271_REG_ID = 0,
         TDA18271_REG_THERMO,
         TDA18271_REG_POWERLEVEL,
         TDA18271_REG_EASYPROG1,
         TDA18271_REG_EASYPROG2,
         TDA18271_REG_EASYPROG3,
         TDA18271_REG_EASYPROG4,
         TDA18271_REG_EASYPROG5,
         TDA18271_REG_CAL_POSTDIV,
         TDA18271_REG_CAL_DIV1,
         TDA18271_REG_CAL_DIV2,
         TDA18271_REG_CAL_DIV3,
         TDA18271_REG_POSTDIV,
         TDA18271_REG_DIV1,
         TDA18271_REG_DIV2,
         TDA18271_REG_DIV3,
         TDA18271_REG_EXT1,
         TDA18271_REG_EXT2,
         TDA18271_REG_EXT3,
         TDA18271_REG_EXT4,
         TDA18271_REG_EXT5,
         TDA18271_REG_EXT6,
         TDA18271_REG_EXT7,
         TDA18271_REG_EXT8,
         TDA18271_REG_EXT9,
         TDA18271_REG_EXT10,
         TDA18271_REG_EXT11,
         TDA18271_REG_EXT12,
         TDA18271_REG_EXT13,
         TDA18271_REG_EXT14,
         TDA18271_REG_EXT15,
         TDA18271_REG_EXT16,
         TDA18271_REG_EXT17,
         TDA18271_REG_EXT18,
         TDA18271_REG_EXT19,
         TDA18271_REG_EXT20,
         TDA18271_REG_EXT21,
         TDA18271_REG_EXT22,
         TDA18271_REG_EXT23,
         TDA18271_NUM_REGS
      };
      
      typedef struct
      {
         uint32_t freq_hz;
         uint8_t postdiv;
         uint8_t div;
      } tda18271_pll_table_entry;
      
      typedef struct
      {
         uint32_t freq_hz;
         uint32_t rf1_default_hz;
         uint32_t rf2_default_hz;
         uint32_t rf3_default_hz;
      } tda18271_rf_band_entry;
      
      typedef struct
      {
         const tda18271_rf_band_entry *band;
         uint32_t rf1;
         uint32_t rf2;
         uint32_t rf3;
         double rf_a1;
         double rf_a2;
         double rf_b1;
         double rf_b2;
      } tda18271_rf_filter_entry;
      
      static const uint8_t NUM_RF_BANDS = 7;
      static const tda18271_rf_band_entry rf_bands[NUM_RF_BANDS];
      tda18271_rf_filter_entry m_rf_filter_curve[NUM_RF_BANDS];
      
      tda18271_version m_version;
      tda18271_mode_t m_mode;
      tda18271_analog_ifc_callback m_analog_cb;
      tda18271_digital_ifc_callback m_digital_cb;
      uint8_t m_regs[TDA18271_NUM_REGS];
      uint8_t m_rfcal_temp;
  
      void initialize(int &error);    
      void write_regs(tda18271_reg_t start, tda18271_reg_t end, int &error);
      void read_regs(tda18271_reg_t start, tda18271_reg_t end, int &error);
      void init_regs(int &error);
      void calc_rf_filter_curve(int &error);
      void rf_tracking_filters_init(tda18271_rf_filter_entry &rf_filter, int &error);
      void powerscan_init(int &error);
      void calc_main_pll(uint32_t freq_hz, int &error);
      void calc_cal_pll(uint32_t freq_hz, int &error);
      void update_rf_band(uint32_t freq_hz, int &error);
      uint8_t get_rf_cal(uint32_t freq_hz, int &error);
      void update_gain_taper(uint32_t freq_hz, int &error);
      void get_cid_target(uint32_t freq_hz, uint8_t &cid_target, uint16_t &count_limit, int &error);
      bool powerscan(uint32_t freq_hz, uint32_t &cal_freq_hz, int &error);
      void update_bp_filter(uint32_t freq_hz, int &error);
      void update_rfc_km(uint32_t freq_hz, int &error);
      uint8_t calibrate_rf(uint32_t freq_hz, int &error);
      uint8_t temperature(int &error);
      void power_on_reset(int &error);
      void rf_tracking_filter_calibration(uint32_t freq_hz, int &error);
      void rf_tracking_filter_correction(uint32_t freq_hz, int &error);
      void update_ir_measure(uint32_t freq_hz, int &error);
      void set_rf(uint32_t freq_hz, tda18271_interface &ifc, int &error);
};

#endif
