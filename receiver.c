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
#include "common.h"

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

  rf12_init(RECEIVER_NODE_ID);

  uint8_t i = 0;
  uint8_t current_node_id = 0x01;
  uint8_t buffer[3];
  for (;;) {
    buffer[0] = POLL_COMMAND;
    buffer[1] = ~POLL_COMMAND;
    rf12_txdata(current_node_id, buffer, 2);
    rf12_rxdata(buffer, 3);
    if (buffer[0] == current_node_id && buffer[2] == (buffer[0] ^ buffer[1])) {
      if (i % 2 == 0) {
	LED_ON();
      } else {
	LED_OFF();
      }
      i += 1;
    }
    _delay_ms(1000);
  }
}
