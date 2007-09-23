CXXFLAGS = -O2 -Wall -fPIC

.if defined(DIAGNOSTIC)
CXXFLAGS+= -D_DIAGNOSTIC
.endif

BASE = pll_driver.o tuner_devnode_device.o tuner_firmware.o tuner_config.o

all: dtt7612 or51132.o
	g++ -o libtuner.so -Wall -O2 -fPIC -shared *.o

dtt7612: $(BASE) dtt7612.h
or51132.o: $(BASE) or51132.h or51132.cpp

pll_driver.o: tuner_driver.h dvb_driver.h pll_driver.h pll_driver.cpp
tuner_device.o: tuner_device.h tuner_devnode_device.h tuner_devnode_device.cpp
tuner_firmware.o: tuner_firmware.h tuner_firmware.cpp
tuner_config.o: tuner_config.h tuner_config.cpp

clean:
	rm -f *.o *.so
