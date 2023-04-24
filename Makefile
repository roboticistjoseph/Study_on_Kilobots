# Authors: Dr. Michael Otte, Joseph Katakam;

# Instructions to convert a '.c' file to '.hex' file
#
# call as follows in the terminal to compile the file 'thisFile.c':
# make FILENAME=thisFile.c
#
# this will create a 'hex' folder in main directory and create a thisFile.hex in it. (path: hex/thisFile.hex)

# getting the file
#
# filename to be parsed
FILENAME ?= blank
# searching the file in the directory
FILE_BASENAME := $(shell find . -name "$(FILENAME)" -print -quit)
# removing the extension (for syntax)
FILE_HOLDER = $(basename $(FILE_BASENAME))
# storing the file_path by removing './' (for syntax)
FILE_PATH = $(subst ./,,$(FILE_HOLDER))
# getting actual file without path
FILE = $(notdir $(FILE_PATH))

# KILOLIB_PACKAGE = kilolib/

# list of all targets to build
all: bootldr $(FILE) ohc ohc-arduino-8mhz ohc-arduino-16mhz
# make PHONY targets to differentiate from actual files
.PHONY: docs bootldr $(FILE) ohc ohc-arduino-8mhz ohc-arduino-16mhz

# KILOLIB variable: holds object files when built
KILOLIB = build/kilolib.a

# list of targets to build
bootldr: build/bootldr.elf build/bootldr.hex build/bootldr.lss
$(FILE): hex build/$(FILE).elf build/$(FILE).hex build/$(FILE).lss hex/$(FILE).hex
ohc: build/ohc.elf build/ohc.hex build/ohc.lss
ohc-arduino-8mhz: build/ohc-arduino-8mhz.elf build/ohc-arduino-8mhz.hex build/ohc-arduino-8mhz.lss
ohc-arduino-16mhz: build/ohc-arduino-16mhz.elf build/ohc-arduino-16mhz.hex build/ohc-arduino-16mhz.lss

# various pre-processor flags for compiler: sets architecture and optimization
CC = avr-gcc
AVRAR = avr-ar
AVROC = avr-objcopy
AVROD = avr-objdump
AVRUP = avrdude

# debugging options
PFLAGS = -P usb -c avrispmkII # user to reprogram OHC

# -03 options
#CFLAGS = -mmcu=atmega328p -Wall -gdwarf-2 -O3 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

# -0s options (i.e., size)
CFLAGS = -mmcu=atmega328p -Wall -gdwarf-2 -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

CFLAGS += -DF_CPU=8000000
# modifying CFLAGS to reduce the size of code generated
# CFLAGS += -DF_CPU=8000000 -Os -fdata-sections -ffunction-sections -fno-move-loop-invariants -fno-tree-loop-optimize -fno-inline-small-functions -fno-reorder-functions -fno-jump-tables -fno-toplevel-reorder -fno-unit-at-a-time
ASFLAGS = $(CFLAGS)
BOOTLDR_FLAGS = -Wl,-section-start=.text=0x7000 -DBOOTLOADER
OHC_FLAGS = -Wl,-section-start=.text=0x7000 -DOHC
OHC_ARDUINO_FLAGS = -DOHC -DARDUINO

FLASH = -R .eeprom -R .fuse -R .lock -R .signature
EEPROM = -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0  

# defining rules for building .elf, .hex, .lss & EEP
%.lss: %.elf
	$(AVROD) -d -S $< > $@

%.hex: %.elf
	$(AVROC) -O ihex $(FLASH) $< $@

%.eep: %.elf
	$(AVROC) -O ihex $(EEPROM) $< $@

%.bin: %.elf
	$(AVROC) -O binary $(FLASH) $< $@ 

# create these directories, if not already present
build:
	mkdir -p $@
hex:
	mkdir -p $@
# docs:
# 	mkdir -p $@

# setting source files
$(KILOLIB): kilolib/kilolib.o kilolib/message_crc.o kilolib/message_send.o | build
	$(AVRAR) rcs $@ kilolib/kilolib.o kilolib/message_crc.o kilolib/message_send.o 
	rm -f *.o

# rules for creating output files
build/$(FILE).elf: $(FILE_PATH).c $(KILOLIB) | build
	$(CC) $(CFLAGS) -o $@ $< $(KILOLIB) 

build/ohc.elf: kilolib/ohc.c kilolib/message_crc.c kilolib/message_send.S | build
	$(CC) $(CFLAGS) $(OHC_FLAGS) -o $@ kilolib/ohc.c kilolib/message_crc.c kilolib/message_send.S

build/ohc-arduino-8mhz.elf: kilolib/ohc.c kilolib/message_crc.c kilolib/message_send.S | build
	$(CC) $(CFLAGS) $(OHC_ARDUINO_FLAGS) -o $@ kilolib/ohc.c kilolib/message_crc.c kilolib/message_send.S

build/ohc-arduino-16mhz.elf: kilolib/ohc.c kilolib/message_crc.c kilolib/message_send.S | build
	$(CC) $(CFLAGS) $(OHC_ARDUINO_FLAGS) -DARDUINO_16MHZ -o $@ kilolib/ohc.c kilolib/message_crc.c kilolib/message_send.S

build/bootldr.elf: kilolib/bootldr.c kilolib/kilolib.c kilolib/message_crc.c | build
	$(CC) $(CFLAGS) $(BOOTLDR_FLAGS) -o $@ kilolib/bootldr.c kilolib/kilolib.c kilolib/message_crc.c

hex/$(FILE).hex: $(FILE_PATH).c $(KILOLIB)
	cp build/$(FILE).hex hex/


# M.Otte: I believe the following is trying upload the hex program to the chip.
#        Therefore, I have commented it out, since the prefered way to do this
#        is with the kilogui program.
#
#program-ohc: build/ohc.hex
#	$(AVRUP) -p m328  $(PFLAGS) -U "flash:w:$<:i"
#
#program-ohc-arduino-8mhz: build/ohc-arduino-8mhz.hex
#	$(AVRUP) -p m328p $(PFLAGS) -U "flash:w:$<:i"
#
#program-ohc-arduino-16mhz: build/ohc-arduino-16mhz.hex
#	$(AVRUP) -p m328p $(PFLAGS) -U "flash:w:$<:i"
#
#program-boot: build/bootldr.hex
#	$(AVRUP) -p m328p $(PFLAGS) -U "flash:w:$<:i"
#
#program-$(FILE): build/$(FILE).hex build/bootldr.hex
#	$(AVRUP) -p m328p $(PFLAGS) -U "flash:w:build/$(FILE).hex:i" -U "flash:w:build/bootldr.hex"


# create Doxygen documentation file
docs:
	cat kilolib/message.h kilolib/kilolib.h kilolib/message_crc.h | grep -v "^\#" > docs/kilolib.h
	(cd docs; doxygen)

# delete build directory
clean:
	rm -fR build
