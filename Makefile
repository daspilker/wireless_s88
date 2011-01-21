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
#CLOCK      = 8000000
CLOCK      = 4000000
BAUD       = 9600
PORT       = USB
#FUSES      = DFE4
FUSES      = DFE2

TRANSMITTER_SOURCES    = transmitter.c rf12.c
TRANSMITTER_OBJECTS    = $(TRANSMITTER_SOURCES:.c=.o)

RECEIVER_SOURCES    = receiver.c rf12.c
RECEIVER_OBJECTS    = $(RECEIVER_SOURCES:.c=.o)

CFLAGS     = -Wall -O2 -mmcu=$(DEVICE) -std=c99
CPPFLAGS   = -DF_CPU=$(CLOCK) -DBAUD=$(BAUD)
CC         = avr-gcc

.PHONY: all
all: transmitter.hex receiver.hex

-include $(TRANSMITTER_SOURCES:.c=.d) $(RECEIVER_SOURCES:.c=.d)

.PHONY: flash-transmitter
flash-transmitter: transmitter.hex
	stk500 -d$(DEVICE) -c$(PORT) -e -iftransmitter.hex -pf -vf

.PHONY: receiver-transmitter
flash-receiver: receiver.hex
	stk500 -d$(DEVICE) -c$(PORT) -e -ifreceiver.hex -pf -vf

.PHONY: fuse
fuse:
	stk500 -d$(DEVICE) -c$(PORT) -f$(FUSES) -F$(FUSES) -EFF -GFF

.PHONY: clean
clean:
	rm -f transmitter.hex transmitter.elf $(TRANSMITTER_OBJECTS) $(TRANSMITTER_SOURCES:.c=.d)
	rm -f receiver.hex receiver.elf $(RECEIVER_OBJECTS) $(RECEIVER_SOURCES:.c=.d)

transmitter.elf: $(TRANSMITTER_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o transmitter.elf $(TRANSMITTER_OBJECTS)

transmitter.hex: transmitter.elf
	avr-objcopy -j .text -j .data -O ihex transmitter.elf transmitter.hex

receiver.elf: $(RECEIVER_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o receiver.elf $(RECEIVER_OBJECTS)

receiver.hex: receiver.elf
	avr-objcopy -j .text -j .data -O ihex receiver.elf receiver.hex

%.d: %.c
	@set -e; $(CC) -MM $(CPPFLAGS) $< -o $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' $@.$$$$ > $@; \
	rm -f $@.$$$$
