#
# Copyright (c) Daniel A. Spilker.
# All rights reserved.
#
# This work is licensed under the Creative Commons Attribution 3.0 Germany License.
# To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/de/.
#
# http://daniel-spilker.com/
#

DEVICE     = attiny2313
CLOCK      = 8000000
BAUD       = 9600
PORT       = USB
FUSES      = DFDC

SOURCES    = transmitter.c rf12.c
OBJECTS    = $(SOURCES:.c=.o)

CFLAGS     = -Wall -O3 -mmcu=$(DEVICE) -std=c99
CPPFLAGS   = -DF_CPU=$(CLOCK) -DBAUD=$(BAUD)
CC         = avr-gcc

.PHONY: all
all: transmitter.hex

-include $(SOURCES:.c=.d)

.PHONY: flash
flash: transmitter.hex
	stk500 -d$(DEVICE) -c$(PORT) -e -iftransmitter.hex -pf -vf

.PHONY: fuse
fuse:
	stk500 -d$(DEVICE) -c$(PORT) -f$(FUSES) -F$(FUSES) -EFF -GFF

.PHONY: clean
clean:
	rm -f transmitter.hex transmitter.elf $(OBJECTS) $(SOURCES:.c=.d)

transmitter.elf: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o transmitter.elf $(OBJECTS)

transmitter.hex: transmitter.elf
	avr-objcopy -j .text -j .data -O ihex transmitter.elf transmitter.hex

%.d: %.c
	@set -e; $(CC) -MM $(CPPFLAGS) $< -o $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' $@.$$$$ > $@; \
	rm -f $@.$$$$
