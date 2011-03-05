#
# Copyright (c) Daniel A. Spilker.
# All rights reserved.
#
# This work is licensed under the Creative Commons Attribution 3.0 Germany License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/de/.
#
# http://daniel-spilker.com/
#

DEVICE     = atmega48a
CLOCK      = 8000000
PORT       = USB
ifeq ($(DEVICE), atmega328p)
  FUSES    = D9E2
else ifeq ($(DEVICE), atmega48a)
  FUSES    = DFE2
endif
EXT_FUSES  = FF

SOURCES    = main.c rf12.c
OBJECTS    = $(SOURCES:.c=.o)

CFLAGS     = -Wall -O3 -mmcu=$(DEVICE) -std=c99
CPPFLAGS   = -DF_CPU=$(CLOCK)
CC         = avr-gcc

.PHONY: all
all: main.hex

-include $(SOURCES:.c=.d)

.PHONY: flash
flash: main.hex
	stk500 -d$(DEVICE) -c$(PORT) -e -ifmain.hex -pf -vf

.PHONY: fuse
fuse:
	stk500 -d$(DEVICE) -c$(PORT) -f$(FUSES) -F$(FUSES) -E$(EXT_FUSES) -G$(EXT_FUSES)

.PHONY: clean
clean:
	rm -f main.hex main.elf $(OBJECTS) $(SOURCES:.c=.d)

.PHONY: size
size: main.elf
	avr-size --format=avr --mcu=$(DEVICE) main.elf

main.elf: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o main.elf $(OBJECTS)

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

%.d: %.c
	@set -e; $(CC) -MM $(CPPFLAGS) $< -o $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' $@.$$$$ > $@; \
	rm -f $@.$$$$
