#export ARDUINO=c:/Programs/arduino-1.0.3
#export ARDUINO=/usr/share/arduino

PRJ = libethercard

MCU = atmega328p 
PRG = arduino

SRC= \
  dhcp.cpp enc28j60.cpp tcpip.cpp webutil.cpp \
  dns.cpp EtherCard.cpp udpserver.cpp

OBJ=$(SRC:.cpp=.o)

CARGS=\
  -c -g -Os -Wall \
  -fno-exceptions -ffunction-sections -fdata-sections \
  -mmcu=atmega328p -DF_CPU=16000000L -MMD \
  -DUSB_VID=null -DUSB_PID=null -DARDUINO=101 \
  -I$(ARDUINO)/hardware/arduino/cores/arduino \
  -I$(ARDUINO)/hardware/arduino/variants/standard \
  -I.

all: $(PRJ).a

clean:
	rm -f *.o *.d $(PRJ).a

%.o: %.cpp
	avr-g++ $(CARGS) $< 

$(PRJ).a: $(OBJ)
	avr-ar cqv $@ $^


