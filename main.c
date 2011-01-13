//
// Copyright (c) Daniel A. Spilker.
// All rights reserved.
//
// This work is licensed under the Creative Commons Attribution 3.0 Germany License.
// To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/de/.
//
// http://daniel-spilker.com/
//

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define PIN_S88_RESET                  PB3
#define PIN_S88_LOAD                   PB4
#define PIN_S88_DATA_IN                PB5
#define PIN_S88_DATA_OUT               PB6
#define PIN_S88_CLOCK                  PB7

ISR(USI_OVERFLOW_vect) {
  USIDR = 0xAA;
  USISR |= _BV(USIOIF);
}

ISR(PCINT_vect) {
  if (bit_is_set(PINB, PIN_S88_LOAD)) {
    USIDR = 0x55;
    USISR = 0x00;
  }
}

static void init() {
  DDRB = _BV(PIN_S88_DATA_OUT);

  USICR = _BV(USIOIE) | _BV(USIWM0) | _BV(USICS1) | _BV(USICS0);

  PCMSK = _BV(PCINT4);
  GIMSK |= _BV(PCIE);

  sei();
}

int main() {
  init();  

  for(;;);
}
