.include "libtuner.ver"

CXXFLAGS ?= -O2 
CXXFLAGS += -Wall

INSTALLDIR ?= /usr/local
DATADIR ?= $(PREFIX)/share/libtuner

.if defined(DIAGNOSTIC)
CXXFLAGS+= -g -D_DIAGNOSTIC
.endif

SRCS = tuner_device.h tuner_device.cpp \
       tuner_driver.h \
       avb_driver.h \
       dvb_driver.h \
       pll_driver.h pll_driver.cpp \
       tuner_devnode_device.h tuner_devnode_device.cpp \
       tuner_firmware.h tuner_firmware.cpp \
       tuner_config.h tuner_config.cpp \
       pll_driver.h pll_driver.cpp \
       tda9887.h tda9887.cpp \
       fmd1216me.h fmd1216me.cpp \
       dtt75105.h dtt75105.cpp \
       dtt7612.h dtt7612.cpp \
       dtt7579.h dtt7579.cpp \
       lgh064f.h lgh064f.cpp \
       or51132.h or51132.cpp \
       lg3303.h lg3303.cpp \
       cx22702.h cx22702.cpp \
       mt2131.h mt2131.cpp \
       cx24227.h cx24227.cpp \
       s5h1411.h s5h1411.cpp \
       xc5000.h xc5000.cpp	\
       tda18271.h tda18271.cpp \
       tda8295.h tda8295.cpp \
       tuv1236d.h tuv1236d.cpp \
       nxt2004.h nxt2004.cpp \
		 xc3028.h xc3028.cpp

NO_PROFILE=
LIB = tuner_static
SHLIB = tuner
SHLIB_MAJOR = $(LIBTUNER_MAJOR)

install: all
	mkdir -p $(INSTALLDIR)/lib/libtuner
	mkdir -p $(INSTALLDIR)/include/libtuner
	mkdir -p $(DATADIR)
	cp -R libtuner.so* $(INSTALLDIR)/lib/libtuner/
	cp libtuner_static.a $(INSTALLDIR)/lib/libtuner/
	cp *.h $(INSTALLDIR)/include/libtuner/
	cp -r firmware/* $(DATADIR)/

.include <bsd.lib.mk>
