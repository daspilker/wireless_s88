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
#include <avr/wdt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "rf12.h"

#define PIN_LED PB0
#define PORT_LED PORTB
#define DDR_LED DDRB

#define ADDRESS_INPUT_MASK 0x7B;
#ifndef DEBUG
#define FEEDBACK_INPUT_MASK 0x0F;
#else
#define FEEDBACK_INPUT_MASK 0x0E;

#define LED_ON()  PORT_LED |=  _BV(PIN_LED)
#define LED_OFF() PORT_LED &= ~_BV(PIN_LED)
#define IS_LED_ON() bit_is_set(PORT_LED, PIN_LED)
#endif

#define RECEIVER_NODE_ID 0x00
#define POLL_COMMAND     0x01

static volatile uint8_t feedback;
static uint8_t node_id;

static void read_feedback();

ISR(PCINT_vect) {
  read_feedback();
}

static void read_feedback() {
  feedback |= ~PINB & FEEDBACK_INPUT_MASK;
}

static void init() {
#ifdef DEBUG
  DDR_LED |= _BV(PIN_LED);

  for (uint8_t i=0; i<3; i++) {
    _delay_ms(50);
    LED_ON();
    _delay_ms(50);
    LED_OFF();
  }
#endif

  PORTD |= ADDRESS_INPUT_MASK;
  PORTB |= FEEDBACK_INPUT_MASK;

  PCMSK = FEEDBACK_INPUT_MASK;
  GIMSK |= _BV(PCIE);

  uint8_t temp_node_id = ~PIND & ADDRESS_INPUT_MASK;
  node_id = (temp_node_id & 0x03) | ((temp_node_id & 0x78) >> 1);

  rf12_init(node_id);

  wdt_enable(WDTO_500MS);

  sei();
}

int main(void) {
  init();

  uint8_t buffer[3];
  for (;;) {
    wdt_reset();
    rf12_rxdata(buffer, 2);
    uint8_t check = ~buffer[0];
    if (buffer[0] == POLL_COMMAND && buffer[1] == check) {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	read_feedback();
	buffer[0] = node_id;
	buffer[1] = feedback;
	buffer[2] = node_id ^ feedback;
	feedback = 0;
      }
      rf12_txdata(RECEIVER_NODE_ID, buffer, 3);
#ifdef DEBUG
      if (IS_LED_ON()) {
	LED_OFF();
      } else {
	LED_ON();
      }
#endif
    }
  }
}
