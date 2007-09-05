CXXFLAGS = -Wall -fPIC
all: base dtt7612 or51132
	g++ -o libtuner.so -Wall -fPIC -shared *.o

dtt7612: dtt7612.h
or51132: or51132.o
or51132.o: or51132.h or51132.cpp
base: pll_driver.o tuner_devnode_device.o tuner_firmware.o tuner_config.o
pll_driver.o: dvb_driver pll_driver.h pll_driver.cpp
dvb_driver: tuner_driver dvb_driver.h
tuner_driver: tuner_driver.h
tuner_device.o: tuner_device.h tuner_devnode_device.h tuner_devnode_device.cpp
tuner_firmware.o: tuner_firmware.h tuner_firmware.cpp
tuner_config.o: tuner_config.h tuner_config.cpp

clean:
	rm -f *.o *.so