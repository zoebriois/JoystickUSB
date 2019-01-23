#AVR-GCC = compilateur -> transforme le programme en un code machine compréhensif pour le microcontrôleur
export CC = avr-gcc

export MCU = atmega328p
export TARGET_ARCH = -mmcu=$(MCU)

export CFLAGS = -Wall -I. -DF_CPU=16000000 -Os #-g
export LDFLAGS = -g $(TARGET_ARCH) -lm -Wl,--gc-sections #      -Os

TARGET = ATM328p
TERM = /dev/ttyACM0
CPPFLAGS = -mmcu=$(MCU)
PGMERISP = -c stk500v1 -b 115200 -P $(TERM)
#Avrdude -> se charge d'envoyer le code machine dans le microcontrôleur au travers du port série du PC
ARVDUDECONF= -C /usr/local/arduino/arduino-0022/hardware/tools/avrdude.conf
export DUDE = /usr/bin/avrdude -F -v -p $(MCU) $(AVRDUDECONF)

C_SRC = $(wildcard *.c)
OBJS = $(C_SRC:.c=.o)

#regle principale: make
all: $(TARGET).hex

#regle de nettoyage: clean
clean:
	rm -f *.o *.hex *.elf

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex $(TARGET).elf eeprom.hex

#regle de chargement du programme: upload
upload: $(TARGET).hex
	stty -F $(TERM) hupcl # reset
	$(DUDE) $(PGMERISP) -U flash:w:$(TARGET).hex
