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
#include <util/delay.h>

#define PIN_LED PD6

int main(void) {
  DDRD |= _BV(PIN_LED);
   
  while (1) {
    PORTD |= _BV(PIN_LED);
    _delay_ms(500);
    PORTD &= ~_BV(PIN_LED);
    _delay_ms(500);
  }

  return 0;
}
