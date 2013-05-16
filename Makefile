#export ARDUINO=c:/Programs/arduino-1.0.3
#export ARDUINO=/usr/share/arduino

# Windows
CFG = $(ARDUINO)/hardware/tools/avr/etc/avrdude.conf
DEV = COM4

# Linux
#DEV = /dev/ttyUSB0
#CFG = /etc/avrdude.conf


MCU = atmega328p 
PRG = arduino
BAUD = 57600

PRJ = udpLedStrip

SRC=$(PRJ).ino

T_OBJ=$(SRC:.ino=.o)
OBJ=$(T_OBJ:.cpp=.o)

CARGS=\
  -c -g -Os -Wall \
  -fno-exceptions -ffunction-sections -fdata-sections \
  -mmcu=atmega328p -DF_CPU=16000000L -MMD \
  -DUSB_VID=null -DUSB_PID=null -DARDUINO=101 \
  -I$(ARDUINO)/hardware/arduino/cores/arduino \
  -I$(ARDUINO)/hardware/arduino/variants/standard \
  -Iethercard \
  -I.

LARGS=-Os -Wl,--gc-sections -mmcu=atmega328p \
  -L. -Lethercard -lm -lethercard -larduino1.0.1_atmega328

all: libethercard $(PRJ).eep $(PRJ).hex

clean:
	rm -f *.o *.d *.elf *.eep *.hex $(PRJ).cpp $(PRJ)

$(PRJ).elf: $(OBJ)
	avr-gcc -o $@ $^ $(LARGS)

$(PRJ).eep: $(PRJ).elf
	avr-objcopy -O ihex -j .eeprom \
      --set-section-flags=.eeprom=alloc,load --no-change-warnings \
      --change-section-lma .eeprom=0 $< $@

$(PRJ).hex: $(PRJ).elf
	avr-objcopy -O ihex -R .eeprom $< $@

%.cpp: %.ino
	echo "#include <Arduino.h>" >$@
	echo "" >>$@
	cat $< >>$@

%.o: %.cpp
	avr-g++ $(CARGS) $< 

libethercard:
	make -C ethercard

# Programming arduino
#   very verbose -> add -v -v -v -v 
load: $(PRJ).hex
	avrdude -C$(CFG) -p$(MCU) -c$(PRG) -P$(DEV) -b$(BAUD) -D -Uflash:w:$<:i



