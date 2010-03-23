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
#include "tda18271.h"

#define TABLE_SIZE(table) (sizeof(table) / sizeof(table[0]))

tda18271::tda18271(
   tuner_config &config,
   tuner_device &device, 
   tda18271_mode_t mode,
   tda18271_analog_ifc_callback analog_cb,
   tda18271_digital_ifc_callback digital_cb,
   int &error)
   : tuner_driver(config, device),
     dvb_driver(config, device),
     avb_driver(config, device),
     m_mode(mode),
     m_analog_cb(analog_cb),
     m_digital_cb(digital_cb)
{
   initialize(error);
}

tda18271::~tda18271(void)
{   
   int error = 0;
   m_regs[TDA18271_REG_EASYPROG3] = (m_regs[TDA18271_REG_EASYPROG3] & 0x1F) | 0xC0;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_EASYPROG3, error);
}

void tda18271::reset(void)
{
   int error = 0;
   initialize(error);
}

void tda18271::stop(void)
{
   int error = 0;
   m_regs[TDA18271_REG_EASYPROG3] = (m_regs[TDA18271_REG_EASYPROG3] & 0x1F) | 0x80;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_EASYPROG3, error);
}

void tda18271::initialize(int &error)
{
   if (error)
   {
      return;
   }
   init_regs(error);
   if (m_version == TDA18271_VER_2)
   {
      calc_rf_filter_curve(error);
      power_on_reset(error);
   }
}

tda18271_version tda18271::get_version(void)
{
   return m_version;
}

void tda18271::write_regs(tda18271_reg_t start, tda18271_reg_t end, int &error)
{
   if (error)
   {
      return;
   }
   if ((end >= TDA18271_NUM_REGS) || (end < start))
   {
      error = EINVAL;
      return;
   }
   uint8_t regbuf[TDA18271_NUM_REGS + 1];
   regbuf[0] = start;
   size_t count = end - start + 1;
   memcpy(regbuf + 1, m_regs + start, count);
   error = m_device.write(regbuf, count + 1);
}
      
void tda18271::read_regs(tda18271_reg_t start, tda18271_reg_t end, int &error)
{
   if (error)
   {
      return;
   }
   if ((end >= TDA18271_NUM_REGS) || (end < start))
   {
      error = EINVAL;
      return;
   }
   tda18271_reg_t regcount = TDA18271_REG_EXT1;
   if (end >= regcount)
   {
      regcount = TDA18271_NUM_REGS;
   }
   uint8_t regbuf[TDA18271_NUM_REGS + 1];
   regbuf[0] = 0x00;
   error = m_device.transact(regbuf, 1, regbuf + 1, regcount);
   if (!error)
   {
      memcpy(m_regs + start, regbuf + start + 1, end - start + 1);
   }
}

void tda18271::init_regs(int &error)
{
   if (error)
   {
      return;
   }
   memset(m_regs, 0x00, TDA18271_NUM_REGS);
   read_regs(TDA18271_REG_ID, TDA18271_REG_ID, error);
   
   // Register init sequence
   m_regs[TDA18271_REG_THERMO] = 0x08;
   m_regs[TDA18271_REG_POWERLEVEL] = 0x80;
   m_regs[TDA18271_REG_EASYPROG1] = 0xC6;
   m_regs[TDA18271_REG_EASYPROG2] = 0xDF;
   m_regs[TDA18271_REG_EASYPROG3] = 0x16;
   m_regs[TDA18271_REG_EASYPROG4] = 0x60;
   m_regs[TDA18271_REG_EASYPROG5] = 0x80;
   m_regs[TDA18271_REG_CAL_POSTDIV] = 0x80;
   m_regs[TDA18271_REG_EXT1] = 0xFF;
   m_regs[TDA18271_REG_EXT2] = 0x01;
   m_regs[TDA18271_REG_EXT3] = 0x84;
   m_regs[TDA18271_REG_EXT4] = 0x41;
   m_regs[TDA18271_REG_EXT5] = 0x01;
   m_regs[TDA18271_REG_EXT6] = 0x84;
   m_regs[TDA18271_REG_EXT7] = 0x40;
   m_regs[TDA18271_REG_EXT8] = 0x07;
   m_regs[TDA18271_REG_EXT11] = 0x96;
   m_regs[TDA18271_REG_EXT12] = 0x0F;
   m_regs[TDA18271_REG_EXT13] = 0xC1;
   m_regs[TDA18271_REG_EXT15] = 0x8F;
   m_regs[TDA18271_REG_EXT20] = 0x20;
   m_regs[TDA18271_REG_EXT21] = 0x33;
   m_regs[TDA18271_REG_EXT22] = 0x48;
   m_regs[TDA18271_REG_EXT23] = 0xB0;
   if (!error)
   {
      switch (m_regs[TDA18271_REG_ID] & 0x7F)
      {
         case 0x3:
            printf("Found tda18271 ver. 1\n");
            m_version = TDA18271_VER_1;
            break;
         case 0x4:
            printf("Found tda18271 ver. 2\n");
            m_version = TDA18271_VER_2;
            m_regs[TDA18271_REG_EXT1] = 0xFC;
            m_regs[TDA18271_REG_EXT12] = 0x33;
            m_regs[TDA18271_REG_EXT18] = 0x8C;
            m_regs[TDA18271_REG_EXT21] = 0xB3;
            break;
         default:
            error = EIO;
            break;
      }
   }
   
   write_regs(TDA18271_REG_THERMO, TDA18271_REG_EXT23, error);
   
   // AGC1 gain sequence
   write_regs(TDA18271_REG_EXT17, TDA18271_REG_EXT17, error);
   m_regs[TDA18271_REG_EXT17] = 0x03;
   write_regs(TDA18271_REG_EXT17, TDA18271_REG_EXT17, error);
   m_regs[TDA18271_REG_EXT17] = 0x43;
   write_regs(TDA18271_REG_EXT17, TDA18271_REG_EXT17, error);
   m_regs[TDA18271_REG_EXT17] = 0x4C;
   write_regs(TDA18271_REG_EXT17, TDA18271_REG_EXT17, error);
   
   if (m_version == TDA18271_VER_1)
   {
      // AGC2 gain sequence
      m_regs[TDA18271_REG_EXT20] = 0xA0;
      write_regs(TDA18271_REG_EXT20, TDA18271_REG_EXT20, error);
      m_regs[TDA18271_REG_EXT20] = 0xA7;
      write_regs(TDA18271_REG_EXT20, TDA18271_REG_EXT20, error);
      m_regs[TDA18271_REG_EXT20] = 0xE7;
      write_regs(TDA18271_REG_EXT20, TDA18271_REG_EXT20, error);
      m_regs[TDA18271_REG_EXT20] = 0xEC;
      write_regs(TDA18271_REG_EXT20, TDA18271_REG_EXT20, error);
   }
   
   // Image rejection cal low-band init sequence
   m_regs[TDA18271_REG_EASYPROG3] = 0x1F;
   m_regs[TDA18271_REG_EASYPROG4] = 0x66;
   m_regs[TDA18271_REG_EASYPROG5] = 0x81;
   m_regs[TDA18271_REG_CAL_POSTDIV] = 0xCC;
   m_regs[TDA18271_REG_CAL_DIV1] = 0x6C;
   m_regs[TDA18271_REG_POSTDIV] = 0xCD;
   m_regs[TDA18271_REG_DIV1] = 0x77;
   m_regs[TDA18271_REG_DIV2] = 0x08;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_DIV3, error);
 
   if (m_version == TDA18271_VER_2)
   {
      // Cycle main charge pump
      m_regs[TDA18271_REG_EXT4] = 0x61;
      write_regs(TDA18271_REG_EXT4, TDA18271_REG_EXT4, error);
      usleep(1000);
      m_regs[TDA18271_REG_EXT4] = 0x41;
      write_regs(TDA18271_REG_EXT4, TDA18271_REG_EXT4, error);
   }
   usleep(5000);
   
   // Launch low-band IR cal
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   usleep(5000);
   m_regs[TDA18271_REG_EASYPROG5] = 0x85;
   m_regs[TDA18271_REG_CAL_POSTDIV] = 0xCB;
   m_regs[TDA18271_REG_CAL_DIV1] = 0x66;
   m_regs[TDA18271_REG_CAL_DIV2] = 0x70;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_CAL_DIV3, error);
   usleep(5000);
   write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
   usleep(30000);
   
   // IR cal mid-band init sequence
   m_regs[TDA18271_REG_EASYPROG5] = 0x82;
   m_regs[TDA18271_REG_CAL_POSTDIV] = 0xA8;
   m_regs[TDA18271_REG_CAL_DIV2] = 0x00;
   m_regs[TDA18271_REG_POSTDIV] = 0xA9;
   m_regs[TDA18271_REG_DIV1] = 0x73;
   m_regs[TDA18271_REG_DIV2] = 0x1A;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_DIV3, error);
   usleep(5000);
   
   // Launch mid-band IR cal
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   usleep(5000);
   m_regs[TDA18271_REG_EASYPROG5] = 0x86;
   m_regs[TDA18271_REG_CAL_POSTDIV] = 0xA8;
   m_regs[TDA18271_REG_CAL_DIV1] = 0x66;
   m_regs[TDA18271_REG_CAL_DIV2] = 0xA0;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_CAL_DIV3, error);
   usleep(5000);
   write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
   usleep(30000);
   
   // IR cal high-band init sequence
   m_regs[TDA18271_REG_EASYPROG5] = 0x83;
   m_regs[TDA18271_REG_CAL_POSTDIV] = 0x98;
   m_regs[TDA18271_REG_CAL_DIV1] = 0x65;
   m_regs[TDA18271_REG_CAL_DIV2] = 0x00;
   m_regs[TDA18271_REG_POSTDIV] = 0x99;
   m_regs[TDA18271_REG_DIV1] = 0x71;
   m_regs[TDA18271_REG_DIV2] = 0xCD;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_DIV3, error);
   usleep(5000);
   
   // Launch high-band IR cal
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   usleep(5000);
   m_regs[TDA18271_REG_EASYPROG5] = 0x87;
   m_regs[TDA18271_REG_CAL_DIV1] = 0x65;
   m_regs[TDA18271_REG_CAL_DIV2] = 0x50;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_CAL_DIV3, error);
   usleep(5000);
   write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
   usleep(30000);
   
   // Return to normal mode & synchronize
   m_regs[TDA18271_REG_EASYPROG4] = 0x64;
   write_regs(TDA18271_REG_EASYPROG4, TDA18271_REG_EASYPROG4, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
}

const tda18271::tda18271_rf_band_entry tda18271::rf_bands[] = 
{
   // freq_hz     rf1_default_hz    rf2_default_hz    rf3_default_hz
   {47900000,     46000000,         0,                0},
   {61100000,     52200000,         0,                0},
   {152600000,    70100000,         136800000,        0},
   {164700000,    156700000,        0,                0},
   {203500000,    186250000,        0,                0},
   {457800000,    230000000,        345000000,        426000000},
   {865000000,    489500000,        697500000,        842000000}
};

void tda18271::calc_rf_filter_curve(int &error)
{
   if (error)
   {
      return;
   }
   usleep(200000);
   powerscan_init(error);
   for (size_t i = 0; i < NUM_RF_BANDS; ++i)
   {
      memset(&m_rf_filter_curve[i], 0, sizeof(m_rf_filter_curve[i]));
      m_rf_filter_curve[i].band = &rf_bands[i];
      rf_tracking_filters_init(m_rf_filter_curve[i], error);
   }
   m_rfcal_temp = temperature(error);
}

void tda18271::rf_tracking_filters_init(tda18271_rf_filter_entry &rf_filter, int &error)
{
   if (error)
   {
      return;
   }
   bool cal;
   uint32_t cprog_cal1, cprog_table1;
   cal = powerscan(rf_filter.band->rf1_default_hz, rf_filter.rf1, error);
   cprog_table1 = get_rf_cal(rf_filter.rf1, error);
   if (cal)
   {
      cprog_cal1 = calibrate_rf(rf_filter.rf1, error);
   }
   else
   {
      cprog_cal1 = cprog_table1;
   }
   rf_filter.rf_b1 = (int32_t)cprog_cal1 - cprog_table1;
   if (rf_filter.band->rf2_default_hz == 0)
   {
      return;
   }
   uint32_t cprog_cal2, cprog_table2;
   cal = powerscan(rf_filter.band->rf2_default_hz, rf_filter.rf2, error);
   cprog_table2 = get_rf_cal(rf_filter.rf2, error);
   if (cal)
   {
      cprog_cal2 = calibrate_rf(rf_filter.rf2, error);
   }
   else
   {
      cprog_cal2 = cprog_table2;
   }
   rf_filter.rf_a1 = 
      (double)((int32_t)cprog_cal2 - cprog_table2 - cprog_cal1 + cprog_table1) /
         (((int32_t)rf_filter.rf2 - rf_filter.rf1) / 1000);
   if (rf_filter.band->rf3_default_hz == 0)
   {
      return;
   }
   uint32_t cprog_cal3, cprog_table3;
   cal = powerscan(rf_filter.band->rf3_default_hz, rf_filter.rf3, error);
   cprog_table3 = get_rf_cal(rf_filter.rf3, error);
   if (cal)
   {
      cprog_cal3 = calibrate_rf(rf_filter.rf3, error);
   }
   else
   {
      cprog_cal3 = cprog_table3;
   }
   rf_filter.rf_a2 =
      (double)((int32_t)cprog_cal3 - cprog_table3 - cprog_cal2 + cprog_table2) /
         (((int32_t)rf_filter.rf3 - rf_filter.rf2) / 1000);
   rf_filter.rf_b2 = (int32_t)cprog_cal2 - cprog_table2;
}

void tda18271::powerscan_init(int &error)
{
   if (error)
   {
      return;
   }
   m_regs[TDA18271_REG_EASYPROG3] = (m_regs[TDA18271_REG_EASYPROG3] & 0xE0) | 0x12;
   m_regs[TDA18271_REG_EASYPROG4] &= 0xE0;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_EASYPROG4, error);
   m_regs[TDA18271_REG_EXT18] &= 0x1C;
   write_regs(TDA18271_REG_EXT18, TDA18271_REG_EXT18, error);
   m_regs[TDA18271_REG_EXT21] &= 0x1C;
   m_regs[TDA18271_REG_EXT21] |= 0x06;
   write_regs(TDA18271_REG_EXT21, TDA18271_REG_EXT23, error);
}

void tda18271::calc_main_pll(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   static const tda18271_pll_table_entry main_pll_table_rev1[] = 
   {
      //frequency(Hz)   post-div  div 
      {32000000,        0x5F,     0xF0},
      {35000000,        0x5E,     0xE0},
      {37000000,        0x5D,     0xD0},
      {41000000,        0x5C,     0xC0},
      {44000000,        0x5B,     0xB0},
      {49000000,        0x5A,     0xA0},
      {54000000,        0x59,     0x90},
      {61000000,        0x58,     0x80},
      {65000000,        0x4F,     0x78},
      {70000000,        0x4E,     0x70},
      {75000000,        0x4D,     0x68},
      {82000000,        0x4C,     0x60},
      {89000000,        0x4B,     0x58},
      {98000000,        0x4A,     0x50},
      {109000000,       0x49,     0x48},
      {123000000,       0x48,     0x40},
      {131000000,       0x3F,     0x3C},
      {141000000,       0x3E,     0x38},
      {151000000,       0x3D,     0x34},
      {164000000,       0x3C,     0x30},
      {179000000,       0x3B,     0x2C},
      {197000000,       0x3A,     0x28},
      {219000000,       0x39,     0x24},
      {246000000,       0x38,     0x20},
      {263000000,       0x2F,     0x1E},
      {282000000,       0x2E,     0x1C},
      {303000000,       0x2D,     0x1A},
      {329000000,       0x2C,     0x18},
      {359000000,       0x2B,     0x16},
      {395000000,       0x2A,     0x14},
      {438000000,       0x29,     0x12},
      {493000000,       0x28,     0x10},
      {526000000,       0x1F,     0x0F},
      {564000000,       0x1E,     0x0E},
      {607000000,       0x1D,     0x0D},
      {658000000,       0x1C,     0x0C},
      {718000000,       0x1B,     0x0B},
      {790000000,       0x1A,     0x0A},
      {877000000,       0x19,     0x09},
      {987000000,       0x18,     0x08}
   };
   static const tda18271_pll_table_entry main_pll_table_rev2[] = 
   {
      //frequency(Hz)   post-div  div 
      {33125000,        0x57,       0xF0},
      {35500000,        0x56,       0xE0},
      {38188000,        0x55,       0xD0},
      {41375000,        0x54,       0xC0},
      {45125000,        0x53,       0xB0},
      {49688000,        0x52,       0xA0},
      {55188000,        0x51,       0x90},
      {62125000,        0x50,       0x80},
      {66250000,        0x47,       0x78},
      {71000000,        0x46,       0x70},
      {76375000,        0x45,       0x68},
      {82750000,        0x44,       0x60},
      {90250000,        0x43,       0x58},
      {99375000,        0x42,       0x50},
      {110375000,       0x41,       0x48},
      {124250000,       0x40,       0x40},
      {132500000,       0x37,       0x3C},
      {142000000,       0x36,       0x38},
      {152750000,       0x35,       0x34},
      {165500000,       0x34,       0x30},
      {180500000,       0x33,       0x2C},
      {198750000,       0x32,       0x28},
      {220750000,       0x31,       0x24},
      {248500000,       0x30,       0x20},
      {265000000,       0x27,       0x1E},
      {284000000,       0x26,       0x1C},
      {305500000,       0x25,       0x1A},
      {331000000,       0x24,       0x18},
      {361000000,       0x23,       0x16},
      {397500000,       0x22,       0x14},
      {441500000,       0x21,       0x12},
      {497000000,       0x20,       0x10},
      {530000000,       0x17,       0x0F},
      {568000000,       0x16,       0x0E},
      {611000000,       0x15,       0x0D},
      {662000000,       0x14,       0x0C},
      {722000000,       0x13,       0x0B},
      {795000000,       0x12,       0x0A},
      {883000000,       0x11,       0x09},
      {994000000,       0x10,       0x08}
   };
   const tda18271_pll_table_entry *table;
   size_t table_size;
   if (m_version == TDA18271_VER_1)
   {
      table = main_pll_table_rev1;
      table_size = TABLE_SIZE(main_pll_table_rev1);
   }
   else
   {
      table = main_pll_table_rev2;
      table_size = TABLE_SIZE(main_pll_table_rev2);
   }
   size_t i;
   for (i = 0; (i < table_size) && (table->freq_hz < freq_hz); ++i, ++table);
   if (i == table_size)
   {
      error = EINVAL;
      return;
   }
   m_regs[TDA18271_REG_POSTDIV] = (m_regs[TDA18271_REG_POSTDIV] & 0x80) | 
      (table->postdiv & 0x7F);
   uint32_t div = ((table->div * (freq_hz / 1000)) << 7) / 125;
   m_regs[TDA18271_REG_DIV1] = (div >> 16) & 0x7F;
   m_regs[TDA18271_REG_DIV2] = (div >> 8) & 0xFF;
   m_regs[TDA18271_REG_DIV3] = div & 0xFF;
   write_regs(TDA18271_REG_POSTDIV, TDA18271_REG_DIV3, error);
}

void tda18271::calc_cal_pll(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   static const tda18271_pll_table_entry cal_pll_table_rev1[] = 
   {
      //frequency(Hz)   post-div  div
      {33000000,        0xDD,     0xD0},
      {36000000,        0xDC,     0xC0},
      {40000000,        0xDB,     0xB0},
      {44000000,        0xDA,     0xA0},
      {49000000,        0xD9,     0x90},
      {55000000,        0xD8,     0x80},
      {63000000,        0xD3,     0x70},
      {67000000,        0xCD,     0x68},
      {73000000,        0xCC,     0x60},
      {80000000,        0xCB,     0x58},
      {88000000,        0xCA,     0x50},
      {98000000,        0xC9,     0x48},
      {110000000,       0xC8,     0x40},
      {126000000,       0xC3,     0x38},
      {135000000,       0xBD,     0x34},
      {147000000,       0xBC,     0x30},
      {160000000,       0xBB,     0x2C},
      {176000000,       0xBA,     0x28},
      {196000000,       0xB9,     0x24},
      {220000000,       0xB8,     0x20},
      {252000000,       0xB3,     0x1C},
      {271000000,       0xAD,     0x1A},
      {294000000,       0xAC,     0x18},
      {321000000,       0xAB,     0x16},
      {353000000,       0xAA,     0x14},
      {392000000,       0xA9,     0x12},
      {441000000,       0xA8,     0x10},
      {505000000,       0xA3,     0x0E},
      {543000000,       0x9D,     0x0D},
      {589000000,       0x9C,     0x0C},
      {642000000,       0x9B,     0x0B},
      {707000000,       0x9A,     0x0A},
      {785000000,       0x99,     0x09},
      {883000000,       0x98,     0x08},
      {1010000000,      0x93,     0x07}
   };
   static const tda18271_pll_table_entry cal_pll_table_rev2[] = 
   {
      //frequency(Hz)   post-div  div
      {33813000,        0xDD,     0xD0},
      {36625000,        0xDC,     0xC0},
      {39938000,        0xDB,     0xB0},
      {43938000,        0xDA,     0xA0},
      {48813000,        0xD9,     0x90},
      {54938000,        0xD8,     0x80},
      {62813000,        0xD3,     0x70},
      {67625000,        0xCD,     0x68},
      {73250000,        0xCC,     0x60},
      {79875000,        0xCB,     0x58},
      {87875000,        0xCA,     0x50},
      {97625000,        0xC9,     0x48},
      {109875000,       0xC8,     0x40},
      {125625000,       0xC3,     0x38},
      {135250000,       0xBD,     0x34},
      {146500000,       0xBC,     0x30},
      {159750000,       0xBB,     0x2C},
      {175750000,       0xBA,     0x28},
      {195250000,       0xB9,     0x24},
      {219750000,       0xB8,     0x20},
      {251250000,       0xB3,     0x1C},
      {270500000,       0xAD,     0x1A},
      {293000000,       0xAC,     0x18},
      {319500000,       0xAB,     0x16},
      {351500000,       0xAA,     0x14},
      {390500000,       0xA9,     0x12},
      {439500000,       0xA8,     0x10},
      {502500000,       0xA3,     0x0E},
      {541000000,       0x9D,     0x0D},
      {586000000,       0x9C,     0x0C},
      {639000000,       0x9B,     0x0B},
      {703000000,       0x9A,     0x0A},
      {781000000,       0x99,     0x09},
      {879000000,       0x98,     0x08}
   };
   const tda18271_pll_table_entry *table;
   size_t table_size;
   if (m_version == TDA18271_VER_1)
   {
      table = cal_pll_table_rev1;
      table_size = TABLE_SIZE(cal_pll_table_rev1);
   }
   else
   {
      table = cal_pll_table_rev2;
      table_size = TABLE_SIZE(cal_pll_table_rev2);
   }
   size_t i;
   for (i = 0; (i < table_size) && (table->freq_hz < freq_hz); ++i, ++table);
   if (i == table_size)
   {
      error = EINVAL;
      return;
   }
   m_regs[TDA18271_REG_CAL_POSTDIV] = table->postdiv;
   uint32_t div = ((table->div * (freq_hz / 1000)) << 7) / 125;
   m_regs[TDA18271_REG_CAL_DIV1] = (div >> 16) & 0x7F;
   m_regs[TDA18271_REG_CAL_DIV2] = (div >> 8) & 0xFF;
   m_regs[TDA18271_REG_CAL_DIV3] = div & 0xFF;
   write_regs(TDA18271_REG_CAL_POSTDIV, TDA18271_REG_CAL_DIV3, error);  
}

void tda18271::update_rf_band(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   uint8_t i;
   for (i = 0; i < TABLE_SIZE(rf_bands) && (rf_bands[i].freq_hz < freq_hz); ++i);
   if (i == TABLE_SIZE(rf_bands))
   {
      error = EINVAL;
      return;
   }
   m_regs[TDA18271_REG_EASYPROG2] = (m_regs[TDA18271_REG_EASYPROG2] & 0x1F) | (i << 5);
}

uint8_t tda18271::get_rf_cal(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return 0;
   }
   typedef struct
   {
      uint32_t freq_hz;
      uint8_t rf_cprog;
   } rf_cal_entry;
   static const rf_cal_entry rf_cal_table_rev1[] = 
   {
      //freq_hz      rf_cprog
      {41000000,     0x1E},
      {43000000,     0x30},
      {45000000,     0x43},
      {46000000,     0x4D},
      {47000000,     0x54},
      {47900000,     0x64},
      {49100000,     0x20},
      {50000000,     0x22},
      {51000000,     0x2A},
      {53000000,     0x32},
      {55000000,     0x35},
      {56000000,     0x3C},
      {57000000,     0x3F},
      {58000000,     0x48},
      {59000000,     0x4D},
      {60000000,     0x58},
      {61100000,     0x5F}
   };
   static const rf_cal_entry rf_cal_table_rev2[] = 
   {
      //freq_hz      rf_cal
      {41000000,     0x0F},
      {43000000,     0x1C},
      {45000000,     0x2F},
      {46000000,     0x39},
      {47000000,     0x40},
      {47900000,     0x50},
      {49100000,     0x16},
      {50000000,     0x18},
      {51000000,     0x20},
      {53000000,     0x28},
      {55000000,     0x2B},
      {56000000,     0x32},
      {57000000,     0x35},
      {58000000,     0x3E},
      {59000000,     0x43},
      {60000000,     0x4E},
      {61100000,     0x55},
      {63000000,     0x0F},
      {64000000,     0x11},
      {65000000,     0x12},
      {66000000,     0x15},
      {67000000,     0x16},
      {68000000,     0x17},
      {70000000,     0x19},
      {71000000,     0x1C},
      {72000000,     0x1D},
      {73000000,     0x1F},
      {74000000,     0x20},
      {75000000,     0x21},
      {76000000,     0x24},
      {77000000,     0x25},
      {78000000,     0x27},
      {80000000,     0x28},
      {81000000,     0x29},
      {82000000,     0x2D},
      {83000000,     0x2E},
      {84000000,     0x2F},
      {85000000,     0x31},
      {86000000,     0x33},
      {87000000,     0x34},
      {88000000,     0x35},
      {89000000,     0x37},
      {90000000,     0x38},
      {91000000,     0x39},
      {93000000,     0x3C},
      {94000000,     0x3E},
      {95000000,     0x3F},
      {96000000,     0x40},
      {97000000,     0x42},
      {99000000,     0x45},
      {100000000,    0x46},
      {102000000,    0x48},
      {103000000,    0x4A},
      {105000000,    0x4D},
      {106000000,    0x4E},
      {107000000,    0x50},
      {108000000,    0x51},
      {110000000,    0x54},
      {111000000,    0x56},
      {112000000,    0x57},
      {113000000,    0x58},
      {114000000,    0x59},
      {115000000,    0x5C},
      {116000000,    0x5D},
      {117000000,    0x5F},
      {119000000,    0x60},
      {120000000,    0x64},
      {121000000,    0x65},
      {122000000,    0x66},
      {123000000,    0x68},
      {124000000,    0x69},
      {125000000,    0x6C},
      {126000000,    0x6D},
      {127000000,    0x6E},
      {128000000,    0x70},
      {129000000,    0x71},
      {130000000,    0x75},
      {131000000,    0x77},
      {132000000,    0x78},
      {133000000,    0x7B},
      {134000000,    0x7E},
      {135000000,    0x81},
      {136000000,    0x82},
      {137000000,    0x87},
      {138000000,    0x88},
      {139000000,    0x8D},
      {140000000,    0x8E},
      {141000000,    0x91},
      {142000000,    0x95},
      {143000000,    0x9A},
      {144000000,    0x9D},
      {145000000,    0xA1},
      {146000000,    0xA2},
      {147000000,    0xA4},
      {148000000,    0xA9},
      {149000000,    0xAE},
      {150000000,    0xB0},
      {151000000,    0xB1},
      {152000000,    0xB7},
      {153000000,    0xBD},
      {154000000,    0x20},
      {155000000,    0x22},
      {156000000,    0x24},
      {157000000,    0x25},
      {158000000,    0x27},
      {159000000,    0x29},
      {160000000,    0x2C},
      {161000000,    0x2D},
      {163000000,    0x2E},
      {164000000,    0x2F},
      {165000000,    0x30},
      {166000000,    0x11},
      {167000000,    0x12},
      {168000000,    0x13},
      {169000000,    0x14},
      {170000000,    0x15},
      {172000000,    0x16},
      {173000000,    0x17},
      {174000000,    0x18},
      {175000000,    0x1A},
      {176000000,    0x1B},
      {178000000,    0x1D},
      {179000000,    0x1E},
      {180000000,    0x1F},
      {181000000,    0x20},
      {182000000,    0x21},
      {183000000,    0x22},
      {184000000,    0x24},
      {185000000,    0x25},
      {186000000,    0x26},
      {187000000,    0x27},
      {188000000,    0x29},
      {189000000,    0x2A},
      {190000000,    0x2C},
      {191000000,    0x2D},
      {192000000,    0x2E},
      {193000000,    0x2F},
      {194000000,    0x30},
      {195000000,    0x33},
      {196000000,    0x35},
      {198000000,    0x36},
      {200000000,    0x38},
      {201000000,    0x3C},
      {202000000,    0x3D},
      {203500000,    0x3E},
      {206000000,    0x0E},
      {208000000,    0x0F},
      {212000000,    0x10},
      {216000000,    0x11},
      {217000000,    0x12},
      {218000000,    0x13},
      {220000000,    0x14},
      {222000000,    0x15},
      {225000000,    0x16},
      {228000000,    0x17},
      {231000000,    0x18},
      {234000000,    0x19},
      {235000000,    0x1A},
      {236000000,    0x1B},
      {237000000,    0x1C},
      {240000000,    0x1D},
      {242000000,    0x1F},
      {247000000,    0x20},
      {249000000,    0x21},
      {252000000,    0x22},
      {253000000,    0x23},
      {254000000,    0x24},
      {256000000,    0x25},
      {259000000,    0x26},
      {262000000,    0x27},
      {264000000,    0x28},
      {267000000,    0x29},
      {269000000,    0x2A},
      {271000000,    0x2B},
      {273000000,    0x2C},
      {275000000,    0x2D},
      {277000000,    0x2E},
      {279000000,    0x2F},
      {282000000,    0x30},
      {284000000,    0x31},
      {286000000,    0x32},
      {287000000,    0x33},
      {290000000,    0x34},
      {293000000,    0x35},
      {295000000,    0x36},
      {297000000,    0x37},
      {300000000,    0x38},
      {303000000,    0x39},
      {305000000,    0x3A},
      {306000000,    0x3B},
      {307000000,    0x3C},
      {310000000,    0x3D},
      {312000000,    0x3E},
      {315000000,    0x3F},
      {318000000,    0x40},
      {320000000,    0x41},
      {323000000,    0x42},
      {324000000,    0x43},
      {325000000,    0x44},
      {327000000,    0x45},
      {331000000,    0x46},
      {334000000,    0x47},
      {337000000,    0x48},
      {339000000,    0x49},
      {340000000,    0x4A},
      {341000000,    0x4B},
      {343000000,    0x4C},
      {345000000,    0x4D},
      {349000000,    0x4E},
      {352000000,    0x4F},
      {353000000,    0x50},
      {355000000,    0x51},
      {357000000,    0x52},
      {359000000,    0x53},
      {361000000,    0x54},
      {362000000,    0x55},
      {364000000,    0x56},
      {368000000,    0x57},
      {370000000,    0x58},
      {372000000,    0x59},
      {375000000,    0x5A},
      {376000000,    0x5B},
      {377000000,    0x5C},
      {379000000,    0x5D},
      {382000000,    0x5E},
      {384000000,    0x5F},
      {385000000,    0x60},
      {386000000,    0x61},
      {388000000,    0x62},
      {390000000,    0x63},
      {393000000,    0x64},
      {394000000,    0x65},
      {396000000,    0x66},
      {397000000,    0x67},
      {398000000,    0x68},
      {400000000,    0x69},
      {402000000,    0x6A},
      {403000000,    0x6B},
      {407000000,    0x6C},
      {408000000,    0x6D},
      {409000000,    0x6E},
      {410000000,    0x6F},
      {411000000,    0x70},
      {412000000,    0x71},
      {413000000,    0x72},
      {414000000,    0x73},
      {417000000,    0x74},
      {418000000,    0x75},
      {420000000,    0x76},
      {422000000,    0x77},
      {423000000,    0x78},
      {424000000,    0x79},
      {427000000,    0x7A},
      {428000000,    0x7B},
      {429000000,    0x7D},
      {432000000,    0x7F},
      {434000000,    0x80},
      {435000000,    0x81},
      {436000000,    0x83},
      {437000000,    0x84},
      {438000000,    0x85},
      {439000000,    0x86},
      {440000000,    0x87},
      {441000000,    0x88},
      {442000000,    0x89},
      {445000000,    0x8A},
      {446000000,    0x8B},
      {447000000,    0x8C},
      {448000000,    0x8E},
      {449000000,    0x8F},
      {450000000,    0x90},
      {452000000,    0x91},
      {453000000,    0x93},
      {454000000,    0x94},
      {456000000,    0x96},
      {457000000,    0x98},
      {461000000,    0x11},
      {468000000,    0x12},
      {472000000,    0x13},
      {473000000,    0x14},
      {474000000,    0x15},
      {481000000,    0x16},
      {486000000,    0x17},
      {491000000,    0x18},
      {498000000,    0x19},
      {499000000,    0x1A},
      {501000000,    0x1B},
      {506000000,    0x1C},
      {511000000,    0x1D},
      {516000000,    0x1E},
      {520000000,    0x1F},
      {521000000,    0x20},
      {525000000,    0x21},
      {529000000,    0x22},
      {533000000,    0x23},
      {539000000,    0x24},
      {541000000,    0x25},
      {547000000,    0x26},
      {549000000,    0x27},
      {551000000,    0x28},
      {556000000,    0x29},
      {561000000,    0x2A},
      {563000000,    0x2B},
      {565000000,    0x2C},
      {569000000,    0x2D},
      {571000000,    0x2E},
      {577000000,    0x2F},
      {580000000,    0x30},
      {582000000,    0x31},
      {584000000,    0x32},
      {588000000,    0x33},
      {591000000,    0x34},
      {596000000,    0x35},
      {598000000,    0x36},
      {603000000,    0x37},
      {604000000,    0x38},
      {606000000,    0x39},
      {612000000,    0x3A},
      {615000000,    0x3B},
      {617000000,    0x3C},
      {621000000,    0x3D},
      {622000000,    0x3E},
      {625000000,    0x3F},
      {632000000,    0x40},
      {633000000,    0x41},
      {634000000,    0x42},
      {642000000,    0x43},
      {643000000,    0x44},
      {647000000,    0x45},
      {650000000,    0x46},
      {652000000,    0x47},
      {657000000,    0x48},
      {661000000,    0x49},
      {662000000,    0x4A},
      {665000000,    0x4B},
      {667000000,    0x4C},
      {670000000,    0x4D},
      {673000000,    0x4E},
      {676000000,    0x4F},
      {677000000,    0x50},
      {681000000,    0x51},
      {683000000,    0x52},
      {686000000,    0x53},
      {688000000,    0x54},
      {689000000,    0x55},
      {691000000,    0x56},
      {695000000,    0x57},
      {698000000,    0x58},
      {703000000,    0x59},
      {704000000,    0x5A},
      {705000000,    0x5B},
      {707000000,    0x5C},
      {710000000,    0x5D},
      {712000000,    0x5E},
      {717000000,    0x5F},
      {718000000,    0x60},
      {721000000,    0x61},
      {722000000,    0x62},
      {723000000,    0x63},
      {725000000,    0x64},
      {727000000,    0x65},
      {730000000,    0x66},
      {732000000,    0x67},
      {735000000,    0x68},
      {740000000,    0x69},
      {741000000,    0x6A},
      {742000000,    0x6B},
      {743000000,    0x6C},
      {745000000,    0x6D},
      {747000000,    0x6E},
      {748000000,    0x6F},
      {750000000,    0x70},
      {752000000,    0x71},
      {754000000,    0x72},
      {757000000,    0x73},
      {758000000,    0x74},
      {760000000,    0x75},
      {763000000,    0x76},
      {764000000,    0x77},
      {766000000,    0x78},
      {767000000,    0x79},
      {768000000,    0x7A},
      {773000000,    0x7B},
      {774000000,    0x7C},
      {776000000,    0x7D},
      {777000000,    0x7E},
      {778000000,    0x7F},
      {779000000,    0x80},
      {781000000,    0x81},
      {783000000,    0x82},
      {784000000,    0x83},
      {785000000,    0x84},
      {786000000,    0x85},
      {793000000,    0x86},
      {794000000,    0x87},
      {795000000,    0x88},
      {797000000,    0x89},
      {799000000,    0x8A},
      {801000000,    0x8B},
      {802000000,    0x8C},
      {803000000,    0x8D},
      {804000000,    0x8E},
      {810000000,    0x90},
      {811000000,    0x91},
      {812000000,    0x92},
      {814000000,    0x93},
      {816000000,    0x94},
      {817000000,    0x96},
      {818000000,    0x97},
      {820000000,    0x98},
      {821000000,    0x99},
      {822000000,    0x9A},
      {828000000,    0x9B},
      {829000000,    0x9D},
      {830000000,    0x9F},
      {831000000,    0xA0},
      {833000000,    0xA1},
      {835000000,    0xA2},
      {836000000,    0xA3},
      {837000000,    0xA4},
      {838000000,    0xA6},
      {840000000,    0xA8},
      {842000000,    0xA9},
      {845000000,    0xAA},
      {846000000,    0xAB},
      {847000000,    0xAD},
      {848000000,    0xAE},
      {852000000,    0xAF},
      {853000000,    0xB0},
      {858000000,    0xB1},
      {860000000,    0xB2},
      {861000000,    0xB3},
      {862000000,    0xB4},
      {863000000,    0xB6},
      {864000000,    0xB8},
      {865000000,    0xB9},
   };
   const rf_cal_entry *table;
   size_t table_size;
   if (m_version == TDA18271_VER_1)
   {
      table = rf_cal_table_rev1;
      table_size = TABLE_SIZE(rf_cal_table_rev1);
   }
   else
   {
      table = rf_cal_table_rev2;
      table_size = TABLE_SIZE(rf_cal_table_rev2);
   }
   size_t i;
   for (i = 0; (i < table_size) && (table->freq_hz < freq_hz); ++i, ++table);
   if (i == table_size)
   {
      error = EINVAL;
      return 0;
   }
   return table->rf_cprog;
}

void tda18271::update_gain_taper(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   typedef struct
   {
      uint32_t freq_hz;
      uint8_t gain_taper;
   } gain_taper_entry;
   static const gain_taper_entry gain_taper_table[] = 
   {
      //freq_hz      gain_taper
      {45400000,     0x1F},
      {45800000,     0x1E},
      {46200000,     0x1D},
      {46700000,     0x1C},
      {47100000,     0x1B},
      {47500000,     0x1A},
      {47900000,     0x19},
      {49600000,     0x17},
      {51200000,     0x16},
      {52900000,     0x15},
      {54500000,     0x14},
      {56200000,     0x13},
      {57800000,     0x12},
      {59500000,     0x11},
      {61100000,     0x10},
      {67600000,     0x0D},
      {74200000,     0x0C},
      {80700000,     0x0B},
      {87200000,     0x0A},
      {93800000,     0x09},
      {100300000,    0x08},
      {106900000,    0x07},
      {113400000,    0x06},
      {119900000,    0x05},
      {126500000,    0x04},
      {133000000,    0x03},
      {139500000,    0x02},
      {146100000,    0x01},
      {152600000,    0x00},
      {154300000,    0x1F},
      {156100000,    0x1E},
      {157800000,    0x1D},
      {159500000,    0x1C},
      {161200000,    0x1B},
      {163000000,    0x1A},
      {164700000,    0x19},
      {170200000,    0x17},
      {175800000,    0x16},
      {181300000,    0x15},
      {186900000,    0x14},
      {192400000,    0x13},
      {198000000,    0x12},
      {203500000,    0x11},
      {216200000,    0x14},
      {228900000,    0x13},
      {241600000,    0x12},
      {254400000,    0x11},
      {267100000,    0x10},
      {279800000,    0x0F},
      {292500000,    0x0E},
      {305200000,    0x0D},
      {317900000,    0x0C},
      {330700000,    0x0B},
      {343400000,    0x0A},
      {356100000,    0x09},
      {368800000,    0x08},
      {381500000,    0x07},
      {394200000,    0x06},
      {406900000,    0x05},
      {419700000,    0x04},
      {432400000,    0x03},
      {445100000,    0x02},
      {457800000,    0x01},
      {476300000,    0x19},
      {494800000,    0x18},
      {513300000,    0x17},
      {531800000,    0x16},
      {550300000,    0x15},
      {568900000,    0x14},
      {587400000,    0x13},
      {605900000,    0x12},
      {624400000,    0x11},
      {642900000,    0x10},
      {661400000,    0x0F},
      {679900000,    0x0E},
      {698400000,    0x0D},
      {716900000,    0x0C},
      {735400000,    0x0B},
      {753900000,    0x0A},
      {772500000,    0x09},
      {791000000,    0x08},
      {809500000,    0x07},
      {828000000,    0x06},
      {846500000,    0x05},
      {865000000,    0x04}
   };
   size_t i;
   for (i = 0; (i < TABLE_SIZE(gain_taper_table)) && (gain_taper_table[i].freq_hz < freq_hz); ++i);
   if (i == TABLE_SIZE(gain_taper_table))
   {
      error = EINVAL;
      return;
   }
   m_regs[TDA18271_REG_EASYPROG2] = (m_regs[TDA18271_REG_EASYPROG2] & 0xE0) | gain_taper_table[i].gain_taper;
}

void tda18271::get_cid_target(uint32_t freq_hz, uint8_t &cid_target, uint16_t &count_limit, int &error)
{
   if (error)
   {
      return;
   }
   typedef struct
   {
      uint32_t freq_hz;
      uint16_t count_limit;
      uint8_t cid_target;
   } cid_target_entry;
   static const cid_target_entry cid_target_table[] =
   {
      //freq_hz      count_limit    cid_target
      {46000000,     1800,          0x04},
      {52200000,     1500,          0x0A},
      {70100000,     4000,          0x01},
      {136800000,    4000,          0x18},
      {156700000,    4000,          0x18},
      {186250000,    4000,          0x0A},
      {230000000,    4000,          0x0A},
      {345000000,    4000,          0x18},
      {426000000,    4000,          0x0E},
      {489500000,    4000,          0x1E},
      {697500000,    4000,          0x32},
      {842000000,    4000,          0x3A}
   };
   size_t i;
   for (i = 0; (i < TABLE_SIZE(cid_target_table)) && (cid_target_table[i].freq_hz < freq_hz); ++i);
   if (i == TABLE_SIZE(cid_target_table))
   {
      error = EINVAL;
      return;
   }
   count_limit = cid_target_table[i].count_limit;
   cid_target = cid_target_table[i].cid_target;
}

bool tda18271::powerscan(uint32_t freq_hz, uint32_t &cal_freq_hz, int &error)
{
   if (error)
   {
      return false;
   }
   update_rf_band(freq_hz, error);
   update_gain_taper(freq_hz, error);
   write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
   m_regs[TDA18271_REG_EXT14] = get_rf_cal(freq_hz, error);
   write_regs(TDA18271_REG_EXT14, TDA18271_REG_EXT14, error);
   
   uint16_t count_limit;
   uint8_t cid_target;
   get_cid_target(freq_hz, cid_target, count_limit, error);
   
   m_regs[TDA18271_REG_EASYPROG4] = (m_regs[TDA18271_REG_EASYPROG4] & 0xFC) | 0x01;
   write_regs(TDA18271_REG_EASYPROG4, TDA18271_REG_EASYPROG4, error);
   cal_freq_hz = freq_hz;
   uint32_t temp_freq;
   int sgn = 1, count = 0;
   bool wait = true;
   do
   {
      temp_freq = freq_hz + (sgn * count) + 1000000;
      calc_main_pll(temp_freq, error);
      if (wait)
      {
         usleep(5000);
         wait = false;
      }
      else
      {
         usleep(100);
      }
      write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
      read_regs(TDA18271_REG_EXT10, TDA18271_REG_EXT10, error);
      count += 200;
      if (count <= count_limit)
      {
         continue;
      }
      else if (sgn == -1)
      {
         break;
      }
      sgn = -1;
      count = 200;
      wait = true;
   } while (!error && ((m_regs[TDA18271_REG_EXT10] & 0x3F) < cid_target));
   if ((m_regs[TDA18271_REG_EXT10] & 0x3F) < cid_target)
   {
      cal_freq_hz = temp_freq - 1000000;
      return true;
   }
   else
   {
      return false;
   }
}

void tda18271::update_bp_filter(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   static const uint32_t bp_filter_table[] = 
      {62000000, 84000000, 100000000, 140000000, 170000000, 180000000, 865000000};
   uint8_t i;
   for (i = 0; (i < TABLE_SIZE(bp_filter_table)) && (bp_filter_table[i] < freq_hz); ++i);
   if (i == TABLE_SIZE(bp_filter_table))
   {
      error = EINVAL;
      return;
   }
   m_regs[TDA18271_REG_EASYPROG1] = (m_regs[TDA18271_REG_EASYPROG1] & 0xF8) | i;
}

void tda18271::update_rfc_km(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   typedef struct
   {
      uint32_t freq_hz;
      uint8_t rfc_km;
   } rfc_km_entry;
   static const rfc_km_entry rfc_km_table_rev1[] =
   {
      // freq_hz     rfc_km
      {61100000,     0x74},
      {350000000,    0x40},
      {720000000,    0x30},
      {865000000,    0x40}
   };
   static const rfc_km_entry rfc_km_table_rev2[] =
   {
      // freq_hz     rfc_km
      {47900000,     0x38},
      {61100000,     0x44},
      {350000000,    0x30},
      {720000000,    0x24},
      {865000000,    0x3C}
   };
   const rfc_km_entry *table;
   size_t table_size;
   if (m_version == TDA18271_VER_1)
   {
      table = rfc_km_table_rev1;
      table_size = TABLE_SIZE(rfc_km_table_rev1);
   }
   else
   {
      table = rfc_km_table_rev2;
      table_size = TABLE_SIZE(rfc_km_table_rev2);
   }
   size_t i;
   for (i = 0; (i < table_size) && (table->freq_hz < freq_hz); ++i, ++table)
   if (i == table_size)
   {
      error = EINVAL;
      return;
   }
   m_regs[TDA18271_REG_EXT13] = (m_regs[TDA18271_REG_EXT13] & 0x83) | table->rfc_km;
}

uint8_t tda18271::calibrate_rf(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return 0;
   }
   m_regs[TDA18271_REG_EASYPROG4] &= 0xFC;
   write_regs(TDA18271_REG_EASYPROG4, TDA18271_REG_EASYPROG4, error);
   m_regs[TDA18271_REG_EXT18] |= 0x03;
   write_regs(TDA18271_REG_EXT18, TDA18271_REG_EXT18, error);
   m_regs[TDA18271_REG_EASYPROG3] |= 0x40;
   update_bp_filter(freq_hz, error);
   update_gain_taper(freq_hz, error);
   update_rf_band(freq_hz, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG3, error);
   update_rfc_km(freq_hz, error);
   write_regs(TDA18271_REG_EXT13, TDA18271_REG_EXT13, error);
   m_regs[TDA18271_REG_EXT4] |= 0x20;
   write_regs(TDA18271_REG_EXT4, TDA18271_REG_EXT4, error);
   m_regs[TDA18271_REG_EXT7] |= 0x20;
   write_regs(TDA18271_REG_EXT7, TDA18271_REG_EXT7, error);
   m_regs[TDA18271_REG_EXT14] = 0x00;
   write_regs(TDA18271_REG_EXT14, TDA18271_REG_EXT14, error);
   m_regs[TDA18271_REG_EXT20] &= 0xDF;
   write_regs(TDA18271_REG_EXT20, TDA18271_REG_EXT20, error);
   m_regs[TDA18271_REG_EASYPROG4] |= 0x03;
   write_regs(TDA18271_REG_EASYPROG4, TDA18271_REG_EASYPROG5, error);
 
   calc_cal_pll(freq_hz, error);
   calc_main_pll(freq_hz + 1000000, error);
   usleep(5000);
   write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   
   m_regs[TDA18271_REG_EXT4] &= 0xDF;
   write_regs(TDA18271_REG_EXT4, TDA18271_REG_EXT4, error);
   m_regs[TDA18271_REG_EXT7] &= 0xDF;
   write_regs(TDA18271_REG_EXT7, TDA18271_REG_EXT7, error);
   usleep(10000);
   
   m_regs[TDA18271_REG_EXT20] |= 0x20;
   write_regs(TDA18271_REG_EXT20, TDA18271_REG_EXT20, error);
   usleep(60000);
   
   m_regs[TDA18271_REG_EXT18] &= 0xFC;
   write_regs(TDA18271_REG_EXT18, TDA18271_REG_EXT18, error);
   m_regs[TDA18271_REG_EASYPROG3] &= 0xBF;
   m_regs[TDA18271_REG_EASYPROG4] &= 0xFC;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_EASYPROG4, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   
   read_regs(TDA18271_REG_EXT14, TDA18271_REG_EXT14, error);
   return m_regs[TDA18271_REG_EXT14];
}

uint8_t tda18271::temperature(int &error)
{
   if (error)
   {
      return 0;
   }
   static const uint8_t thermometer_table[16][2] = 
   {
      {60,  92},
      {62,  94},
      {66,  98},
      {64,  96},
      {74,  106},
      {72,  104},
      {68,  100},
      {70,  102},
      {90,  122},
      {88,  120},
      {84,  116},
      {86,  118},
      {76,  108},
      {78,  110},
      {82,  114},
      {80,  112}
   };
   m_regs[TDA18271_REG_THERMO] |= 0x10;
   write_regs(TDA18271_REG_THERMO, TDA18271_REG_THERMO, error);
   read_regs(TDA18271_REG_THERMO, TDA18271_REG_THERMO, error);
   uint8_t temp_range = m_regs[TDA18271_REG_THERMO] & 0x20;
   uint8_t temp = m_regs[TDA18271_REG_THERMO] & 0x0F;
   if (((temp_range == 0x20) && (temp == 0x00)) ||
       ((temp_range == 0x00) && (temp == 0x08)))
   {
      m_regs[TDA18271_REG_THERMO] = (m_regs[TDA18271_REG_THERMO] & 0xDF) | ((~temp_range) & 0x20);
      write_regs(TDA18271_REG_THERMO, TDA18271_REG_THERMO, error);
      usleep(10000);
      read_regs(TDA18271_REG_THERMO, TDA18271_REG_THERMO, error);
   }
   m_regs[TDA18271_REG_THERMO] &= 0xEF;
   write_regs(TDA18271_REG_THERMO, TDA18271_REG_THERMO, error);
   m_regs[TDA18271_REG_EASYPROG4] &= 0xFC;
   write_regs(TDA18271_REG_EASYPROG4, TDA18271_REG_EASYPROG4, error);
   return thermometer_table[temp][temp_range >> 5];
}

void tda18271::power_on_reset(int &error)
{
   if (error)
   {
      return;
   }
   m_regs[TDA18271_REG_EXT12] &= 0xDF;
   write_regs(TDA18271_REG_EXT12, TDA18271_REG_EXT12, error);
   m_regs[TDA18271_REG_EXT18] &= 0x7C;
   write_regs(TDA18271_REG_EXT18, TDA18271_REG_EXT18, error);
   m_regs[TDA18271_REG_EASYPROG3] = (m_regs[TDA18271_REG_EASYPROG3] & 0x1F) | 0x80;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_EASYPROG3, error);
   m_regs[TDA18271_REG_EXT21] |= 0x03;
   m_regs[TDA18271_REG_EXT23] &= 0xF9;
   write_regs(TDA18271_REG_EXT21, TDA18271_REG_EXT23, error);
}

void tda18271::rf_tracking_filter_calibration(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   update_bp_filter(freq_hz, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   m_regs[TDA18271_REG_EXT4] = (m_regs[TDA18271_REG_EXT4] & 0x07) | 0x60;
   write_regs(TDA18271_REG_EXT4, TDA18271_REG_EXT4, error);
   m_regs[TDA18271_REG_EXT7] = 0x60;
   write_regs(TDA18271_REG_EXT7, TDA18271_REG_EXT7, error);
   m_regs[TDA18271_REG_EXT14] = 0x00;
   write_regs(TDA18271_REG_EXT14, TDA18271_REG_EXT14, error);
   m_regs[TDA18271_REG_EXT20] = 0xCC;
   write_regs(TDA18271_REG_EXT20, TDA18271_REG_EXT20, error);
   m_regs[TDA18271_REG_EASYPROG4] |= 0x03;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_EASYPROG5, error);
   calc_cal_pll(freq_hz, error);
   calc_main_pll(freq_hz + 1000000, error);
   usleep(5000);
   update_rfc_km(freq_hz, error);
   write_regs(TDA18271_REG_EXT13, TDA18271_REG_EXT13, error);
   update_rf_band(freq_hz, error);
   update_gain_taper(freq_hz, error);
   write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   write_regs(TDA18271_REG_EASYPROG2, TDA18271_REG_EASYPROG2, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   m_regs[TDA18271_REG_EXT4] = (m_regs[TDA18271_REG_EXT4] & 0x07) | 0x40;
   write_regs(TDA18271_REG_EXT4, TDA18271_REG_EXT4, error);
   m_regs[TDA18271_REG_EXT7] = 0x40;
   write_regs(TDA18271_REG_EXT7, TDA18271_REG_EXT7, error);
   usleep(10000);
   m_regs[TDA18271_REG_EXT20] = 0xEC;
   write_regs(TDA18271_REG_EXT20, TDA18271_REG_EXT20, error);
   usleep(60000);
   m_regs[TDA18271_REG_EASYPROG4] &= 0xFC;
   write_regs(TDA18271_REG_EASYPROG4, TDA18271_REG_EASYPROG4, error);
   write_regs(TDA18271_REG_EASYPROG1, TDA18271_REG_EASYPROG1, error);
   if (freq_hz <= 61100000)
   {
      uint8_t rf_cal = get_rf_cal(freq_hz, error);
      m_regs[TDA18271_REG_EXT14] = rf_cal;
      write_regs(TDA18271_REG_EXT14, TDA18271_REG_EXT14, error);
   }
}

void tda18271::rf_tracking_filter_correction(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   typedef struct
   {
      uint32_t freq_hz;
      uint8_t rfc_diff;
   } rfc_differential_entry;
   static const rfc_differential_entry rfc_differential_table[] =
   {
      // freq_hz     rfc_diff
      {47900000,     0x00},
      {55000000,     0x00},
      {61100000,     0x0A},
      {64000000,     0x0A},
      {82000000,     0x14},
      {84000000,     0x19},
      {119000000,    0x1C},
      {124000000,    0x20},
      {129000000,    0x2A},
      {134000000,    0x32},
      {139000000,    0x39},
      {144000000,    0x3E},
      {149000000,    0x3F},
      {152600000,    0x40},
      {154000000,    0x40},
      {164700000,    0x41},
      {203500000,    0x32},
      {353000000,    0x19},
      {356000000,    0x1A},
      {359000000,    0x1B},
      {363000000,    0x1C},
      {366000000,    0x1D},
      {369000000,    0x1E},
      {373000000,    0x1F},
      {376000000,    0x20},
      {379000000,    0x21},
      {383000000,    0x22},
      {386000000,    0x23},
      {389000000,    0x24},
      {393000000,    0x25},
      {396000000,    0x26},
      {399000000,    0x27},
      {402000000,    0x28},
      {404000000,    0x29},
      {407000000,    0x2A},
      {409000000,    0x2B},
      {412000000,    0x2C},
      {414000000,    0x2D},
      {417000000,    0x2E},
      {419000000,    0x2F},
      {422000000,    0x30},
      {424000000,    0x31},
      {427000000,    0x32},
      {429000000,    0x33},
      {432000000,    0x34},
      {434000000,    0x35},
      {437000000,    0x36},
      {439000000,    0x37},
      {442000000,    0x38},
      {444000000,    0x39},
      {447000000,    0x3A},
      {449000000,    0x3B},
      {457800000,    0x3C},
      {465000000,    0x0F},
      {477000000,    0x12},
      {483000000,    0x14},
      {502000000,    0x19},
      {508000000,    0x1B},
      {519000000,    0x1C},
      {522000000,    0x1D},
      {524000000,    0x1E},
      {534000000,    0x1F},
      {549000000,    0x20},
      {554000000,    0x22},
      {584000000,    0x24},
      {589000000,    0x26},
      {658000000,    0x27},
      {664000000,    0x2C},
      {669000000,    0x2D},
      {699000000,    0x2E},
      {704000000,    0x30},
      {709000000,    0x31},
      {714000000,    0x32},
      {724000000,    0x33},
      {729000000,    0x36},
      {739000000,    0x38},
      {744000000,    0x39},
      {749000000,    0x3B},
      {754000000,    0x3C},
      {759000000,    0x3D},
      {764000000,    0x3E},
      {769000000,    0x3F},
      {774000000,    0x40},
      {779000000,    0x41},
      {784000000,    0x43},
      {789000000,    0x46},
      {794000000,    0x48},
      {799000000,    0x4B},
      {804000000,    0x4F},
      {809000000,    0x54},
      {814000000,    0x59},
      {819000000,    0x5D},
      {824000000,    0x61},
      {829000000,    0x68},
      {834000000,    0x6E},
      {839000000,    0x75},
      {844000000,    0x7E},
      {849000000,    0x82},
      {854000000,    0x84},
      {859000000,    0x8F},
      {865000000,    0x9A}
   };
   m_regs[TDA18271_REG_EASYPROG3] &= 0x1F;
   write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_EASYPROG3, error);
   uint8_t rf_cal = get_rf_cal(freq_hz, error);
   tda18271_rf_filter_entry *filter = m_rf_filter_curve;
   size_t i;
   for (i = 0; (i < NUM_RF_BANDS) && (filter->band->freq_hz < freq_hz); ++i, ++filter);
   if (i == NUM_RF_BANDS)
   {
      error = EINVAL;
      return;
   }
   double capprox;
   if ((filter->rf3 == 0) || (freq_hz < filter->rf2))
   {
      capprox = filter->rf_a1 * ((freq_hz - filter->rf1) / 1000) + filter->rf_b1 + rf_cal;
   }
   else
   {
      capprox = filter->rf_a2 * ((freq_hz - filter->rf2) / 1000) + filter->rf_b2 + rf_cal;
   }
   if (capprox < 0)
   {
      capprox = 0;
   }
   else if (capprox > 255)
   {
      capprox = 255;
   }
   const rfc_differential_entry *diff = rfc_differential_table;
   for (i = 0; (i < TABLE_SIZE(rfc_differential_table)) && (diff->freq_hz < freq_hz); ++i, ++diff);
   if (i == TABLE_SIZE(rfc_differential_table))
   {
      error = EINVAL;
      return;
   }
   int32_t temp = temperature(error);
   m_regs[TDA18271_REG_EXT14] = (uint8_t)capprox + (((temp - m_rfcal_temp) * diff->rfc_diff) / 1000);
   write_regs(TDA18271_REG_EXT14, TDA18271_REG_EXT14, error);
}

void tda18271::update_ir_measure(uint32_t freq_hz, int &error)
{
   if (error)
   {
      return;
   }
   typedef struct
   {
      uint32_t freq_hz;
      uint8_t ir_measure;
   } ir_measure_entry;
   static const ir_measure_entry ir_measure_table[] = 
   {
      //freq_hz      ir_measure
      {200000000,    0x05},
      {600000000,    0x06},
      {865000000,    0x07}
   };
   size_t i;
   for (i = 0; (i < TABLE_SIZE(ir_measure_table)) && (ir_measure_table[i].freq_hz < freq_hz); ++i);
   if (i == TABLE_SIZE(ir_measure_table))
   {
      error = EINVAL;
      return;
   }
   m_regs[TDA18271_REG_EASYPROG5] = (m_regs[TDA18271_REG_EASYPROG5] & 0xF8) | ir_measure_table[i].ir_measure;
}

void tda18271::set_rf(uint32_t freq_hz, tda18271_interface &ifc, int &error)
{
   if (error)
   {
      return;
   }
   if (m_version == TDA18271_VER_1)
   {
      rf_tracking_filter_calibration(freq_hz, error);
   }
   else
   {
      rf_tracking_filter_correction(freq_hz, error);
   }
   m_regs[TDA18271_REG_EASYPROG3] = (m_regs[TDA18271_REG_EASYPROG3] & 0xE0) |
      (ifc.agc_mode << 3) | ifc.std;
   if (m_version == TDA18271_VER_2)
   {
      m_regs[TDA18271_REG_EASYPROG3] &= 0xFB;
   }
   m_regs[TDA18271_REG_EASYPROG4] = (m_regs[TDA18271_REG_EASYPROG4] & 0x60) | 
      (ifc.fm_rfn << 7) | (ifc.if_level << 2);
      
   m_regs[TDA18271_REG_EXT22] = ifc.rf_agc_top;
   write_regs(TDA18271_REG_EXT22, TDA18271_REG_EXT22, error);
   m_regs[TDA18271_REG_EASYPROG1] |= 0x40;
   m_regs[TDA18271_REG_THERMO] &= 0xE0;
   
   update_ir_measure(freq_hz, error);
   update_bp_filter(freq_hz, error);
   update_rf_band(freq_hz, error);
   update_gain_taper(freq_hz, error);
   
   m_regs[TDA18271_REG_EXT1] &= 0xF8;
   if (m_mode != TDA18271_MODE_SLAVE)
   {
      m_regs[TDA18271_REG_EXT1] |= 0x04;
   }
   write_regs(TDA18271_REG_EXT1, TDA18271_REG_EXT1, error);
   
   uint32_t pll_freq = ifc.ifreq_hz + freq_hz;
   m_regs[TDA18271_REG_POSTDIV] = ifc.if_notch << 7;
   if (m_mode == TDA18271_MODE_SLAVE)
   {
      calc_cal_pll(pll_freq, error);
      m_regs[TDA18271_REG_POSTDIV] |= (m_regs[TDA18271_REG_CAL_POSTDIV] & 0x7F);
      write_regs(TDA18271_REG_POSTDIV, TDA18271_REG_POSTDIV, error);
      write_regs(TDA18271_REG_THERMO, TDA18271_REG_EASYPROG5, error);
      m_regs[TDA18271_REG_EXT7] |= 0x20;
      write_regs(TDA18271_REG_EXT7, TDA18271_REG_EXT7, error);
      usleep(1000);
      m_regs[TDA18271_REG_EXT7] &= 0xDF;
      write_regs(TDA18271_REG_EXT7, TDA18271_REG_EXT7, error);
   }
   else
   {
      calc_main_pll(pll_freq, error);
      write_regs(TDA18271_REG_THERMO, TDA18271_REG_EASYPROG5, error);
      m_regs[TDA18271_REG_EXT4] |= 0x20;
      write_regs(TDA18271_REG_EXT4, TDA18271_REG_EXT4, error);
      usleep(1000);
      m_regs[TDA18271_REG_EXT4] &= 0xDF;
      write_regs(TDA18271_REG_EXT4, TDA18271_REG_EXT4, error);
   }
   usleep(20000);
   if (m_version == TDA18271_VER_2)
   {
      m_regs[TDA18271_REG_EASYPROG3] &= 0xFB;
      if (!ifc.fm_rfn)
      {
         m_regs[TDA18271_REG_EASYPROG3] |= 0x04;
      }
      write_regs(TDA18271_REG_EASYPROG3, TDA18271_REG_EASYPROG3, error);
   }
}

int tda18271::set_channel(const avb_channel &channel)
{
   int error = 0;
   tda18271_interface ifc;
   ifc.if_level = 0;
   ifc.if_notch = 0;
   ifc.rf_agc_top = 0x2C;
   switch (channel.video_format)
   {
      case AVB_VIDEO_FMT_NTSC_M:
      case AVB_VIDEO_FMT_NTSC_N:
      case AVB_VIDEO_FMT_NTSC_J:
      case AVB_VIDEO_FMT_PAL_N:
      case AVB_VIDEO_FMT_PAL_NC:
      case AVB_VIDEO_FMT_PAL_M:
         if (m_version == TDA18271_VER_1)
         {
            ifc.ifreq_hz = 5750000;
            ifc.std = 0x05;
         }
         else
         {
            ifc.ifreq_hz = 5400000;
            ifc.std = 0x04;
         }
         ifc.agc_mode = 1;
         ifc.fm_rfn = 0;
         break;
      case AVB_VIDEO_FMT_PAL_B:
      case AVB_VIDEO_FMT_SECAM_B:
         if (m_version == TDA18271_VER_1)
         {
            ifc.ifreq_hz = 6750000;
            ifc.std = 0x06;
         }
         else
         {
            ifc.ifreq_hz = 6000000;
            ifc.std = 0x05;
         }
         ifc.agc_mode = 1;
         ifc.fm_rfn = 0;
         break;
      case AVB_VIDEO_FMT_PAL_D:
      case AVB_VIDEO_FMT_PAL_D1:
      case AVB_VIDEO_FMT_SECAM_D:
      case AVB_VIDEO_FMT_SECAM_K:
      case AVB_VIDEO_FMT_SECAM_K1:
      case AVB_VIDEO_FMT_SECAM_L:
         if (m_version == TDA18271_VER_1)
         {
            ifc.ifreq_hz = 7750000;
            ifc.std = 0x07;
         }
         else
         {
            ifc.ifreq_hz = 6900000;
            ifc.std = 0x06;
         }
         ifc.agc_mode = 1;
         ifc.fm_rfn = 0;
         break;
      case AVB_VIDEO_FMT_PAL_G:
      case AVB_VIDEO_FMT_PAL_H:
      case AVB_VIDEO_FMT_SECAM_G:
      case AVB_VIDEO_FMT_SECAM_H:
         if (m_version == TDA18271_VER_1)
         {
            ifc.ifreq_hz = 7750000;
            ifc.std = 0x07;
         }
         else
         {
            ifc.ifreq_hz = 7100000;
            ifc.std = 0x06;
         }
         ifc.agc_mode = 1;
         ifc.fm_rfn = 0;
         break;
      case AVB_VIDEO_FMT_PAL_I:
         if (m_version == TDA18271_VER_1)
         {
            ifc.ifreq_hz = 7750000;
            ifc.std = 0x07;
         }
         else
         {
            ifc.ifreq_hz = 7250000;
            ifc.std = 0x06;
         }
         ifc.agc_mode = 1;
         ifc.fm_rfn = 0;
         break;
      case AVB_VIDEO_FMT_SECAM_LC:
         ifc.ifreq_hz = 1250000;
         if (m_version == TDA18271_VER_1)
         {
            ifc.std = 0x07;
         }
         else
         {
            ifc.std = 0x06;
         }
         ifc.agc_mode = 1;
         ifc.fm_rfn = 0;
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
               ifc.ifreq_hz = 1250000;
               ifc.std = 0x00;
               ifc.agc_mode = 3;
               ifc.fm_rfn = 1;
               break;
            default:
               return EINVAL;
         }
         break;
      default:
         return EINVAL;
   }
   if (!error && (m_analog_cb != NULL))
   {
      error = m_analog_cb(*this, channel, ifc);
   }
   set_rf((uint32_t)channel.frequency_hz, ifc, error);
   return error;
}

int tda18271::set_channel(const dvb_channel &channel, dvb_interface &interface)
{
   int error = 0;
   tda18271_interface ifc;
   ifc.if_level = 1;
   ifc.if_notch = 1;
   ifc.rf_agc_top = 0x37;
   ifc.agc_mode = 3;
   ifc.fm_rfn = 0;
   switch (channel.modulation)
   {
      case DVB_MOD_VSB_8:
      case DVB_MOD_VSB_16:
         ifc.ifreq_hz = 3250000;
         ifc.std = 0x04;
         break;
      case DVB_MOD_OFDM:
         switch (channel.bandwidth_hz)
         {
            case 6000000:
               ifc.ifreq_hz = 3300000;
               ifc.std = 0x04;
               break;
            case 7000000:
               if (m_version == TDA18271_VER_1)
               {
                  ifc.ifreq_hz = 3800000;
                  ifc.std = 0x05;
               }
               else
               {
                  ifc.ifreq_hz = 3500000;
                  ifc.std = 0x04;
               }
               break;
            case 8000000:
               if (m_version == TDA18271_VER_1)
               {
                  ifc.ifreq_hz = 4300000;
                  ifc.std = 0x06;
               }
               else
               {
                  ifc.ifreq_hz = 4000000;
                  ifc.std = 0x05;
               }
               break;
            default:
               return EINVAL;
         }
         break;
      case DVB_MOD_QAM_16:
      case DVB_MOD_QAM_32:
      case DVB_MOD_QAM_64:
      case DVB_MOD_QAM_128:
      case DVB_MOD_QAM_256:
      case DVB_MOD_QAM_AUTO:
         switch (channel.bandwidth_hz)
         {
            case 6000000:
               ifc.ifreq_hz = 4000000;
               ifc.std = 0x05;
               break;
            case 8000000:
               ifc.ifreq_hz = 5000000;
               ifc.std = 0x07;
               break;
            default:
               return EINVAL;
         }
         break;
      default:
         return EINVAL;
   }
   if (!error && (m_digital_cb != NULL))
   {
      error = m_digital_cb(*this, channel, ifc);
   }
   set_rf((uint32_t)channel.frequency_hz, ifc, error);
   return error;
}

int tda18271::get_signal(dvb_signal &signal)
{
   return 0;
}

int tda18271::start(uint32_t timeout_ms)
{
   return 0;
}
