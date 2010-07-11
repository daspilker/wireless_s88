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
#include "rf12.h"

#define PIN_LED PD6

#define LED_ON()  PORTD |=  _BV(PIN_LED)
#define LED_OFF() PORTD &= ~_BV(PIN_LED)

int main(void) {
  DDRD |= _BV(PIN_LED);

  for (uint8_t i=0; i<3; i++) {
    _delay_ms(50);
    LED_ON();
    _delay_ms(50);
    LED_OFF();
  }

  rf12_init(0x00);

  uint8_t i = 0;
  uint8_t data[1];
  for (;;) {
    rf12_rxdata(data, 1);
    while (!rf12_can_send());
    rf12_txdata(0x01, data, 1);
    i += 1;
    if (i % 2 == 0) {
      LED_ON();
    } else {
      LED_OFF();
    }
  }
}
