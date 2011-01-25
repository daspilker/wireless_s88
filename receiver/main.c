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
#include <util/delay.h>
#include "rf12.h"

#define POLL_COMMAND 0x01
#define NODE_ID      0x00

#define PIN_LED  PD7
#define PORT_LED PORTD
#define DDR_LED  DDRD

#define LED_ON()  PORT_LED |=  _BV(PIN_LED)
#define LED_OFF() PORT_LED &= ~_BV(PIN_LED)
#define TOGGLE_LED() {if(bit_is_set(PORT_LED, PIN_LED)) LED_OFF(); else LED_ON();}

/*
ISR(SPI_STC_vect) {
  SPDR = 0x55;
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
*/

static void init() {
  DDR_LED |= _BV(PIN_LED);

  for (uint8_t i=0; i<5; i++) {
    LED_ON();
    _delay_ms(50);
    LED_OFF();
    _delay_ms(50);
  }

  //  PCMSK0 = _BV(PCINT1);
  //  PCICR |= _BV(PCIE0);

  //  DDRB |= _BV(PB4);
  //  SPCR = _BV(SPIE) | _BV(SPE) | _BV(CPHA);

  rf12_init(NODE_ID);

  sei();
}

int main() {
  init();

  uint8_t current_node_id = 0x01;
  uint8_t buffer[3];
  for (;;) {
    buffer[0] = POLL_COMMAND;
    buffer[1] = ~POLL_COMMAND;    
    rf12_txdata(current_node_id, buffer, 2);
    if (rf12_rxdata_timeout(buffer, 3)) {
      if (buffer[0] == current_node_id && buffer[2] == (buffer[0] ^ buffer[1])) {
	TOGGLE_LED();
      }
      _delay_ms(50);
    }
  }
}
