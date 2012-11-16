#!/usr/local/bin/python

""" 
  Copyright 2012 Jason Harmening
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
 
  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.
"""

import sys
import struct
import os
import argparse

XC3028_FW_TYPE_BASE =   0
XC3028_FW_TYPE_DVB =    1
XC3028_FW_TYPE_AVB =    2
XC3028_FW_TYPE_SCODE =  3
XC3028_FW_TYPE_MAIN =   4

XC3028_BASEFW_8MHZ =    1 << 0
XC3028_BASEFW_FM =      1 << 1
XC3028_BASEFW_INPUT1 =  1 << 2
XC3028_BASEFW_MTS =     1 << 3

DVB_MOD_UNKNOWN =       0
DVB_MOD_VSB_8 =         1
DVB_MOD_VSB_16 =        2
DVB_MOD_QAM_16 =        3
DVB_MOD_QAM_32 =        4
DVB_MOD_QAM_64 =        5
DVB_MOD_QAM_128 =       6
DVB_MOD_QAM_256 =       7
DVB_MOD_QAM_AUTO =      8
DVB_MOD_QPSK =          9
DVB_MOD_OFDM =          10
DVB_MOD_QPSK_NBC =      11
DVB_MOD_8PSK =          12
DVB_MOD_16PSK =         13

XC3028_DVBFW_6MHZ =     1 << 0
XC3028_DVBFW_7MHZ =     1 << 1
XC3028_DVBFW_8MHZ =     1 << 2
XC3028_DVBFW_78MHZ =    1 << 3
XC3028_DVBFW_D2620 =    1 << 4
XC3028_DVBFW_D2633 =    1 << 5

AVB_VIDEO_FMT_NONE =     0
AVB_VIDEO_FMT_NTSC_M =   1
AVB_VIDEO_FMT_NTSC_N =   2
AVB_VIDEO_FMT_NTSC_J =   3
AVB_VIDEO_FMT_NTSC_443 = 4
AVB_VIDEO_FMT_PAL_B =    5
AVB_VIDEO_FMT_PAL_D =    6
AVB_VIDEO_FMT_PAL_D1 =   7
AVB_VIDEO_FMT_PAL_G =    8
AVB_VIDEO_FMT_PAL_H =    9
AVB_VIDEO_FMT_PAL_I =    10
AVB_VIDEO_FMT_PAL_K =    11
AVB_VIDEO_FMT_PAL_N =    12
AVB_VIDEO_FMT_PAL_NC =   13
AVB_VIDEO_FMT_PAL_M =    14
AVB_VIDEO_FMT_PAL_60 =   15
AVB_VIDEO_FMT_SECAM_L =  16
AVB_VIDEO_FMT_SECAM_LC = 17
AVB_VIDEO_FMT_SECAM_B =  18
AVB_VIDEO_FMT_SECAM_D =  19
AVB_VIDEO_FMT_SECAM_G =  20
AVB_VIDEO_FMT_SECAM_H =  21
AVB_VIDEO_FMT_SECAM_K =  22
AVB_VIDEO_FMT_SECAM_K1 = 23
AVB_VIDEO_FMT_SECAM_K3 = 24

AVB_AUDIO_FMT_NONE =              0
AVB_AUDIO_FMT_AUTO =              1
AVB_AUDIO_FMT_BTSC =              2
AVB_AUDIO_FMT_EIAJ =              3
AVB_AUDIO_FMT_A2 =                4
AVB_AUDIO_FMT_NICAM =             5
AVB_AUDIO_FMT_FM_MONO =           6
AVB_AUDIO_FMT_FM_MONO_NON_USA =   7
AVB_AUDIO_FMT_FM_MONO_USA =       8
AVB_AUDIO_FMT_FM_STEREO =         9
AVB_AUDIO_FMT_FM_STEREO_NON_USA = 10
AVB_AUDIO_FMT_FM_STEREO_USA =     11
AVB_AUDIO_FMT_AM_MONO =           12
AVB_AUDIO_FMT_AM_STEREO =         13
AVB_AUDIO_FMT_BTSC_SAP =          14
AVB_AUDIO_FMT_EIAJ_SAP =          15
AVB_AUDIO_FMT_A2_SAP =            16
AVB_AUDIO_FMT_NICAM_SAP =         17

XC3028_AVBFW_MTS =      1 << 0
XC3028_AVBFW_LCD =      1 << 1
XC3028_AVBFW_NOGD =     1 << 2

XC3028_SCFW_MONO =           1 << 0
XC3028_SCFW_ATSC =           1 << 1
XC3028_SCFW_IF =             1 << 2
XC3028_SCFW_LG60 =           1 << 3
XC3028_SCFW_ATI638 =         1 << 4
XC3028_SCFW_OREN538 =        1 << 5
XC3028_SCFW_OREN36 =         1 << 6
XC3028_SCFW_TOYOTA388 =      1 << 7
XC3028_SCFW_TOYOTA794 =      1 << 8
XC3028_SCFW_DIBCOM52 =       1 << 9
XC3028_SCFW_ZARLINK456 =     1 << 10
XC3028_SCFW_CHINA =          1 << 11
XC3028_SCFW_F6MHZ =          1 << 12
XC3028_SCFW_INPUT2 =         1 << 13

header = {
   'version': 0x2700,
   'base': [
      {
         'offset': 6336,
         'size': 8718,
         'flags': XC3028_BASEFW_8MHZ
      },
      {
         'offset': 15056,
         'size': 8712,
         'flags': XC3028_BASEFW_8MHZ | XC3028_BASEFW_MTS
      },
      {
         'offset': 23776,
         'size': 8562,
         'flags': XC3028_BASEFW_FM
      },
      {
         'offset': 32344,
         'size': 8576,
         'flags': XC3028_BASEFW_FM | XC3028_BASEFW_INPUT1
      },
      {
         'offset': 40928,
         'size': 8706,
         'flags': 0
      },
      {
         'offset': 49640,
         'size': 8682,
         'flags': XC3028_BASEFW_MTS
      }
   ],
   'dvb': [
      {
         'offset': 61080,
         'size': 149,
         'modulation': 1 << DVB_MOD_VSB_8,
         'flags': XC3028_DVBFW_6MHZ | XC3028_DVBFW_D2633
      },
      {
         'offset': 61240,
         'size': 149,
         'modulation': (1 << DVB_MOD_OFDM) | (1 << DVB_MOD_QAM_64) | (1 << DVB_MOD_QAM_AUTO),
         'flags': XC3028_DVBFW_6MHZ | XC3028_DVBFW_D2620
      },
      {
         'offset': 61392,
         'size': 149,
         'modulation': (1 << DVB_MOD_OFDM) | (1 << DVB_MOD_QAM_64) | (1 << DVB_MOD_QAM_AUTO),
         'flags': XC3028_DVBFW_6MHZ | XC3028_DVBFW_D2633
      },
      {
         'offset': 61552,
         'size': 149,
         'modulation': 1 << DVB_MOD_OFDM,
         'flags': XC3028_DVBFW_7MHZ | XC3028_DVBFW_8MHZ | XC3028_DVBFW_D2620
      },
      {
         'offset': 61704,
         'size': 149,
         'modulation': 1 << DVB_MOD_OFDM,
         'flags': XC3028_DVBFW_7MHZ | XC3028_DVBFW_8MHZ | XC3028_DVBFW_D2633
      },
      {
         'offset': 61864,
         'size': 149,
         'modulation': 1 << DVB_MOD_OFDM,
         'flags': XC3028_DVBFW_78MHZ | XC3028_DVBFW_D2620
      },
      {
         'offset': 62016,
         'size': 149,
         'modulation': 1 << DVB_MOD_OFDM,
         'flags': XC3028_DVBFW_78MHZ | XC3028_DVBFW_D2633
      },
   ],
   'avb': [
      {
         'offset': 58328,
         'size': 161,
         'video': (1 << AVB_VIDEO_FMT_PAL_B) | (1 << AVB_VIDEO_FMT_PAL_G),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': 0
      },
      {
         'offset': 58496,
         'size': 169,
         'video': (1 << AVB_VIDEO_FMT_PAL_B) | (1 << AVB_VIDEO_FMT_PAL_G),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 59016,
         'size': 161,
         'video': (1 << AVB_VIDEO_FMT_PAL_B) | (1 << AVB_VIDEO_FMT_PAL_G),
         'audio': (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP),
         'flags': 0
      },
      {
         'offset': 59184,
         'size': 169,
         'video': (1 << AVB_VIDEO_FMT_PAL_B) | (1 << AVB_VIDEO_FMT_PAL_G),
         'audio': (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 59704,
         'size': 161,
         'video': (1 << AVB_VIDEO_FMT_PAL_D) | (1 << AVB_VIDEO_FMT_PAL_D1) | (1 << AVB_VIDEO_FMT_PAL_K),
         'audio': (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': 0
      },
      {
         'offset': 59872,
         'size': 169,
         'video': (1 << AVB_VIDEO_FMT_PAL_D) | (1 << AVB_VIDEO_FMT_PAL_D1) | (1 << AVB_VIDEO_FMT_PAL_K),
         'audio': (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 60048,
         'size': 161,
         'video': (1 << AVB_VIDEO_FMT_PAL_D) | (1 << AVB_VIDEO_FMT_PAL_D1) | (1 << AVB_VIDEO_FMT_PAL_K),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP),
         'flags': 0
      },
      {
         'offset': 60216,
         'size': 169,
         'video': (1 << AVB_VIDEO_FMT_PAL_D) | (1 << AVB_VIDEO_FMT_PAL_D1) | (1 << AVB_VIDEO_FMT_PAL_K),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 60392,
         'size': 161,
         'video': (1 << AVB_VIDEO_FMT_SECAM_K) | (1 << AVB_VIDEO_FMT_SECAM_K1),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP) |
                  (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP) | (1 << AVB_AUDIO_FMT_AM_MONO) | (1 << AVB_AUDIO_FMT_AM_STEREO),
         'flags': 0
      },
      {
         'offset': 60560,
         'size': 169,
         'video': (1 << AVB_VIDEO_FMT_SECAM_K) | (1 << AVB_VIDEO_FMT_SECAM_K1),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP) |
                  (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP) | (1 << AVB_AUDIO_FMT_AM_MONO) | (1 << AVB_AUDIO_FMT_AM_STEREO),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 60736,
         'size': 161,
         'video': 1 << AVB_VIDEO_FMT_SECAM_K3,
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP) |
                  (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP) | (1 << AVB_AUDIO_FMT_AM_MONO) | (1 << AVB_AUDIO_FMT_AM_STEREO),
         'flags': 0
      },
      {
         'offset': 60904,
         'size': 169,
         'video': 1 << AVB_VIDEO_FMT_SECAM_K3,
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP) |
                  (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP) | (1 << AVB_AUDIO_FMT_AM_MONO) | (1 << AVB_AUDIO_FMT_AM_STEREO),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 62488,
         'size': 135,
         'video': 0,
         'audio': (1 << AVB_AUDIO_FMT_FM_MONO) | (1 << AVB_AUDIO_FMT_FM_MONO_NON_USA) | (1 << AVB_AUDIO_FMT_FM_MONO_USA) |
                  (1 << AVB_AUDIO_FMT_FM_STEREO) | (1 << AVB_AUDIO_FMT_FM_STEREO_NON_USA) | (1 << AVB_AUDIO_FMT_FM_STEREO_USA),
         'flags': 0
      },
      {
         'offset': 62632,
         'size': 161,
         'video': 1 << AVB_VIDEO_FMT_PAL_I,
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP) |
                  (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': 0
      },
      {
         'offset': 62800,
         'size': 169,
         'video': 1 << AVB_VIDEO_FMT_PAL_I,
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP) |
                  (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 62976,
         'size': 169,
         'video': 1 << AVB_VIDEO_FMT_SECAM_L,
         'audio': (1 << AVB_AUDIO_FMT_AM_MONO) | (1 << AVB_AUDIO_FMT_AM_STEREO),
         'flags': 0
      },
      {
         'offset': 63152,
         'size': 161,
         'video': 1 << AVB_VIDEO_FMT_SECAM_L,
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP),
         'flags': 0
      },
      {
         'offset': 63320,
         'size': 161,
         'video': 1 << AVB_VIDEO_FMT_SECAM_LC,
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_NICAM) | (1 << AVB_AUDIO_FMT_NICAM_SAP) |
                  (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP) | (1 << AVB_AUDIO_FMT_AM_MONO) | (1 << AVB_AUDIO_FMT_AM_STEREO),
         'flags': 0
      },
      {
         'offset': 63488,
         'size': 161,
         'video': 1 << AVB_VIDEO_FMT_NTSC_M,
         'audio': (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': 0
      },
      {
         'offset': 63656,
         'size': 161,
         'video': 1 << AVB_VIDEO_FMT_NTSC_M,
         'audio': (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': XC3028_AVBFW_LCD
      },
      {
         'offset': 63824,
         'size': 161,
         'video': 1 << AVB_VIDEO_FMT_NTSC_M,
         'audio': (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': XC3028_AVBFW_LCD | XC3028_AVBFW_NOGD
      },
      {
         'offset': 63992,
         'size': 169,
         'video': 1 << AVB_VIDEO_FMT_NTSC_M,
         'audio': (1 << AVB_AUDIO_FMT_A2) | (1 << AVB_AUDIO_FMT_A2_SAP),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 64168,
         'size': 161,
         'video': (1 << AVB_VIDEO_FMT_NTSC_M) | (1 << AVB_VIDEO_FMT_NTSC_N) | (1 << AVB_VIDEO_FMT_NTSC_J) |
                  (1 << AVB_VIDEO_FMT_PAL_M) | (1 << AVB_VIDEO_FMT_PAL_N) | (1 << AVB_VIDEO_FMT_PAL_NC),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_BTSC) | (1 << AVB_AUDIO_FMT_BTSC_SAP) |
                  (1 << AVB_AUDIO_FMT_EIAJ) | (1 << AVB_AUDIO_FMT_EIAJ_SAP),
         'flags': 0
      },
      {
         'offset': 64336,
         'size': 161,
         'video': (1 << AVB_VIDEO_FMT_NTSC_M) | (1 << AVB_VIDEO_FMT_NTSC_N) |
                  (1 << AVB_VIDEO_FMT_PAL_M) | (1 << AVB_VIDEO_FMT_PAL_N) | (1 << AVB_VIDEO_FMT_PAL_NC),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_BTSC) | (1 << AVB_AUDIO_FMT_BTSC_SAP),
         'flags': XC3028_AVBFW_LCD
      },
      {
         'offset': 64504,
         'size': 161,
         'video': (1 << AVB_VIDEO_FMT_NTSC_M) | (1 << AVB_VIDEO_FMT_NTSC_N) |
                  (1 << AVB_VIDEO_FMT_PAL_M) | (1 << AVB_VIDEO_FMT_PAL_N) | (1 << AVB_VIDEO_FMT_PAL_NC),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_BTSC) | (1 << AVB_AUDIO_FMT_BTSC_SAP),
         'flags': XC3028_AVBFW_LCD | XC3028_AVBFW_NOGD
      },
      {
         'offset': 64840,
         'size': 169,
         'video': (1 << AVB_VIDEO_FMT_NTSC_M) | (1 << AVB_VIDEO_FMT_NTSC_N) |
                  (1 << AVB_VIDEO_FMT_PAL_M) | (1 << AVB_VIDEO_FMT_PAL_N) | (1 << AVB_VIDEO_FMT_PAL_NC),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_BTSC) | (1 << AVB_AUDIO_FMT_BTSC_SAP),
         'flags': XC3028_AVBFW_MTS
      },
      {
         'offset': 65016,
         'size': 169,
         'video': (1 << AVB_VIDEO_FMT_NTSC_M) | (1 << AVB_VIDEO_FMT_NTSC_N) |
                  (1 << AVB_VIDEO_FMT_PAL_M) | (1 << AVB_VIDEO_FMT_PAL_N) | (1 << AVB_VIDEO_FMT_PAL_NC),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_BTSC) | (1 << AVB_AUDIO_FMT_BTSC_SAP),
         'flags': XC3028_AVBFW_MTS | XC3028_AVBFW_LCD
      },
      {
         'offset': 65192,
         'size': 169,
         'video': (1 << AVB_VIDEO_FMT_NTSC_M) | (1 << AVB_VIDEO_FMT_NTSC_N) |
                  (1 << AVB_VIDEO_FMT_PAL_M) | (1 << AVB_VIDEO_FMT_PAL_N) | (1 << AVB_VIDEO_FMT_PAL_NC),
         'audio': (1 << AVB_AUDIO_FMT_AUTO) | (1 << AVB_AUDIO_FMT_BTSC) | (1 << AVB_AUDIO_FMT_BTSC_SAP),
         'flags': XC3028_AVBFW_MTS | XC3028_AVBFW_LCD | XC3028_AVBFW_NOGD
      }
   ],
   'scode': [
      {
         'offset': 4800,
         'size': 192,
         'ifreq_khz': 3280,
         'flags': 0
      },
      {
         'offset': 5952,
         'size': 192,
         'ifreq_khz': 3300,
         'flags': 0
      },
      {
         'offset': 5184,
         'size': 192,
         'ifreq_khz': 3440,
         'flags': 0
      },
      {
         'offset': 5376,
         'size': 192,
         'ifreq_khz': 3460,
         'flags': 0
      },
      {
         'offset': 2688,
         'size': 192,
         'ifreq_khz': 3800,
         'flags': XC3028_SCFW_ATSC | XC3028_SCFW_OREN36
      },
      {
         'offset': 4992,
         'size': 192,
         'ifreq_khz': 4000,
         'flags': 0
      },
      {
         'offset': 2880,
         'size': 192,
         'ifreq_khz': 4080,
         'flags': XC3028_SCFW_ATSC | XC3028_SCFW_TOYOTA388
      },
      {
         'offset': 4608,
         'size': 192,
         'ifreq_khz': 4200,
         'flags': 0
      },
      {
         'offset': 960,
         'size': 192,
         'ifreq_khz': 4320,
         'flags': XC3028_SCFW_MONO
      },
      {
         'offset': 5568,
         'size': 192,
         'ifreq_khz': 4450,
         'flags': 0
      },
      {
         'offset': 576,
         'size': 192,
         'ifreq_khz': 4500,
         'flags': XC3028_SCFW_MONO | XC3028_SCFW_IF
      },
      {
         'offset': 768,
         'size': 192,
         'ifreq_khz': 4600,
         'flags': XC3028_SCFW_IF
      },
      {
         'offset': 192,
         'size': 192,
         'ifreq_khz': 4760,
         'flags': XC3028_SCFW_ZARLINK456
      },
      {
         'offset': 4416,
         'size': 192,
         'ifreq_khz': 4940,
         'flags': 0 
      },
      {
         'offset': 3456,
         'size': 192,
         'ifreq_khz': 5260,
         'flags': 0 
      },
      {
         'offset': 3648,
         'size': 192,
         'ifreq_khz': 5320,
         'flags': XC3028_SCFW_MONO 
      },
      {
         'offset': 0,
         'size': 192,
         'ifreq_khz': 5400,
         'flags': XC3028_SCFW_DIBCOM52 | XC3028_SCFW_CHINA
      },
      {
         'offset': 2496,
         'size': 192,
         'ifreq_khz': 5580,
         'flags': XC3028_SCFW_ATSC | XC3028_SCFW_OREN538
      },
      {
         'offset': 1344,
         'size': 192,
         'ifreq_khz': 5640,
         'flags': 0
      },
      {
         'offset': 1536,
         'size': 192,
         'ifreq_khz': 5740,
         'flags': 0
      },
      {
         'offset': 3264,
         'size': 192,
         'ifreq_khz': 5900,
         'flags': 0
      },
      {
         'offset': 1728,
         'size': 192,
         'ifreq_khz': 6000,
         'flags': XC3028_SCFW_MONO
      },
      {
         'offset': 2304,
         'size': 192,
         'ifreq_khz': 6200,
         'flags': XC3028_SCFW_ATSC | XC3028_SCFW_LG60 | XC3028_SCFW_F6MHZ
      },
      {
         'offset': 1152,
         'size': 192,
         'ifreq_khz': 6240,
         'flags': 0
      },
      {
         'offset': 4224,
         'size': 192,
         'ifreq_khz': 6320,
         'flags': XC3028_SCFW_MONO
      },
      {
         'offset': 2112,
         'size': 192,
         'ifreq_khz': 6340,
         'flags': 0
      },
      {
         'offset': 4032,
         'size': 192,
         'ifreq_khz': 6500,
         'flags': XC3028_SCFW_MONO
      },
      {
         'offset': 384,
         'size': 192,
         'ifreq_khz': 6580,
         'flags': XC3028_SCFW_ATSC | XC3028_SCFW_ATI638
      },
      {
         'offset': 1920,
         'size': 192,
         'ifreq_khz': 6600,
         'flags': 0
      },
      {
         'offset': 3840,
         'size': 192,
         'ifreq_khz': 6680,
         'flags': XC3028_SCFW_MONO
      },
      {
         'offset': 3072,
         'size': 192,
         'ifreq_khz': 8140,
         'flags': XC3028_SCFW_ATSC | XC3028_SCFW_TOYOTA794
      },
      {
         'offset': 5760,
         'size': 192,
         'ifreq_khz': 8200,
         'flags': 0
      }
   ]
}

def checkfw(rawfile, header):
   if (header['offset'] + header['size']) > os.path.getsize(rawfile):
      raise ValueError('header at offset %d extends beyond end of file' % header['offset'])

def buildfw(rawfile, outfile):
   print 'Generating annotated firmware %s from raw firmware %s...' % (outfile, rawfile)
   with open(rawfile, 'rb') as rawfw, open(outfile, 'wb') as fwfile:
      fwfile.write(struct.pack('<HHH', header['version'], XC3028_FW_TYPE_BASE, len(header['base'])))
      for base_fw in header['base']:
         checkfw(rawfile, base_fw)
         fwfile.write(struct.pack('<IIH', base_fw['offset'], base_fw['size'], base_fw['flags']))
      fwfile.write(struct.pack('<HH', XC3028_FW_TYPE_DVB, len(header['dvb'])))
      for dvb_fw in header['dvb']:
         checkfw(rawfile, dvb_fw)
         fwfile.write(struct.pack('<IIHH', dvb_fw['offset'], dvb_fw['size'], dvb_fw['modulation'], dvb_fw['flags']))
      fwfile.write(struct.pack('<HH', XC3028_FW_TYPE_AVB, len(header['avb'])))
      for avb_fw in header['avb']:
         checkfw(rawfile, avb_fw)
         fwfile.write(struct.pack('<IIIIH', avb_fw['offset'], avb_fw['size'], avb_fw['video'], avb_fw['audio'], avb_fw['flags']))
      fwfile.write(struct.pack('<HH', XC3028_FW_TYPE_SCODE, len(header['scode'])))
      for sc_fw in header['scode']:
         checkfw(rawfile, sc_fw)
         fwfile.write(struct.pack('<IIHH', sc_fw['offset'], sc_fw['size'], sc_fw['ifreq_khz'], sc_fw['flags']))
      fwfile.write(struct.pack('<HH', XC3028_FW_TYPE_MAIN, 1))
      fwfile.write(rawfw.read())
   print 'done.'

def extractfw(builtfile, outfile):
   print 'Extracting raw firmware %s from annotated firmware %s...' % (outfile, builtfile)
   with open(builtfile, 'rb') as builtfw, open(outfile, 'wb') as rawfw:
      print 'firmware version: 0x%x' % struct.unpack('<H', builtfw.read(2))[0]
      fwdata = {XC3028_FW_TYPE_BASE: ('base', 10),
                XC3028_FW_TYPE_DVB: ('DVB', 12),
                XC3028_FW_TYPE_AVB: ('AVB', 18),
                XC3028_FW_TYPE_SCODE: ('SCODE', 12)}
      while True:
         fwtype, numfws = struct.unpack('<HH', builtfw.read(4))
         if fwtype == XC3028_FW_TYPE_MAIN:
            break
         fwname, hdrsize = fwdata[fwtype]
         print '%d %s firmware images' % (numfws, fwname)
         builtfw.seek(numfws * hdrsize, 1)
      rawfw.write(builtfw.read())
   print 'done.'

if __name__ == "__main__":
   parser = argparse.ArgumentParser()
   parser.add_argument('-i', dest='infile', required=True)
   parser.add_argument('-o', dest='outfile', required=True)
   parser.add_argument('-e', dest='extract', action='store_true')
   args = parser.parse_args()
   if args.extract:
      extractfw(args.infile, args.outfile)
   else:
      buildfw(args.infile, args.outfile)

