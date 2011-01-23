//
// Copyright (c) Daniel A. Spilker.
// All rights reserved.
//
// This work is licensed under the Creative Commons Attribution 3.0 Germany License.
// To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/de/.
//
// http://daniel-spilker.com/
//

#include <avr/io.h>
#include <avr/interrupt.h>

ISR(SPI_STC_vect) {
  SPDR = 0x55;

  if (bit_is_set(PORTC, PC1)) {
    PORTC &= ~_BV(PC1);
  } else {
    PORTC |= _BV(PC1);
  }
}

ISR(PCINT0_vect) {
  if (bit_is_set(PINB, PB1)) {
    SPCR &= ~_BV(SPE);
    SPCR |= _BV(SPE);
    SPDR = 0xAA;
    
    if (bit_is_set(PORTC, PC0)) {
      PORTC &= ~_BV(PC0);
    } else {
      PORTC |= _BV(PC0);
    }
  }
}

int main() {
  DDRC |= _BV(PC0) | _BV(PC1);

  PCMSK0 = _BV(PCINT1);
  PCICR |= _BV(PCIE0);

  DDRB |= _BV(PB4);
  SPCR = _BV(SPIE) | _BV(SPE) | _BV(CPHA);

  sei();

  for(;;);
}
