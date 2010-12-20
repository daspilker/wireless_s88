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

static volatile uint8_t feedback;

int main(void) {
  DDRD |= _BV(PIN_LED);

  for (uint8_t i=0; i<3; i++) {
    _delay_ms(50);
    LED_ON();
    _delay_ms(50);
    LED_OFF();
  }

  uint8_t node_id = 0x01;

  rf12_init(node_id);

  uint8_t i = 0;
  uint8_t buffer[3];
  for (;;) {
    rf12_rxdata(buffer, 2);
    uint8_t check = ~buffer[0];
    if (buffer[0] == POLL_COMMAND && buffer[1] == check) {
      buffer[0] = node_id;
      buffer[1] = feedback;
      buffer[2] = node_id ^ feedback;
      rf12_txdata(RECEIVER_NODE_ID, buffer, 3);
      i += 1;
      if (i % 2 == 0) {
	LED_ON();
      } else {
	LED_OFF();
      }
    }
  }
}
