CXXFLAGS = -O2 -Wall -fPIC

.if defined(DIAGNOSTIC)
CXXFLAGS+= -D_DIAGNOSTIC
.endif

BASE = pll_driver.o tuner_devnode_device.o tuner_firmware.o tuner_config.o
DRIVERS = dtt7612.o lgh064f.o or51132.o lg3303.o

all: $(BASE) $(DRIVERS)
	g++ -o libtuner.so -Wall -O2 -fPIC -shared *.o

dtt7612.o: pll_driver.o dtt7612.h dtt7612.cpp
lgh064f.o: pll_driver.o lgh064f.h lgh064f.cpp
or51132.o: tuner_device.h tuner_config.o tuner_firmware.o or51132.h or51132.cpp
lg3303.o: tuner_device.h tuner_config.o lg3303.h lg3303.cpp

pll_driver.o: tuner_config.o tuner_device.h tuner_driver.h dvb_driver.h pll_driver.h pll_driver.cpp
tuner_devnode_device.o: tuner_device.h tuner_devnode_device.h tuner_devnode_device.cpp
tuner_firmware.o: tuner_firmware.h tuner_firmware.cpp
tuner_config.o: tuner_config.h tuner_config.cpp

clean:
	rm -f *.o *.so
