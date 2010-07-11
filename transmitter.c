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

  rf12_init(0x01);

  uint8_t data_tx[] = "a";
  uint8_t data_rx[8];
  for (;;) {
    while (!rf12_can_send());
    rf12_txdata(0x00, data_tx, 1);
    rf12_rxdata(data_rx, 1);
    if (data_rx[0] == data_tx[0]) {
      LED_ON();
    } else {
      LED_OFF();
    }
    data_tx[0] += 1;
    _delay_ms(1000);
  }
}
