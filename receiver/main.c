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

#define PIN_S88LOAD     PB1
#define PIN_S88DATA_OUT PB4

#define MODULE_COUNT_MASK_PORTC (_BV(PC5) | _BV(PC4) | _BV(PC3) | _BV(PC2) | _BV(PC1) | _BV(PC0))
#define MODULE_COUNT_MASK_PORTD (_BV(PC6) | _BV(PC5))

#define LED_ON()  PORT_LED |=  _BV(PIN_LED)
#define LED_OFF() PORT_LED &= ~_BV(PIN_LED)

volatile uint8_t feedback = 0;

ISR(SPI_STC_vect) {
  SPDR = 0x00;
}

ISR(PCINT0_vect) {
  if (bit_is_set(PINB, PIN_S88LOAD)) {
    SPCR &= ~_BV(SPE);
    SPCR |= _BV(SPE);
    SPDR = feedback;
  }
}

static uint8_t init() {
  uint8_t transmitter_count;

  DDR_LED |= _BV(PIN_LED);

  for (uint8_t i=0; i<5; i++) {
    LED_ON();
    _delay_ms(50);
    LED_OFF();
    _delay_ms(50);
  }

  PCMSK0 = _BV(PCINT1);
  PCICR |= _BV(PCIE0);

  DDRB |= _BV(PIN_S88DATA_OUT);
  SPCR = _BV(SPIE) | _BV(SPE) | _BV(CPHA);

  PORTC |= MODULE_COUNT_MASK_PORTC;
  PORTD |= MODULE_COUNT_MASK_PORTD;

  transmitter_count = ~PINC & MODULE_COUNT_MASK_PORTC;
  transmitter_count |= (~PIND & MODULE_COUNT_MASK_PORTD) << 2;

  rf12_init(NODE_ID);

  sei();

  return transmitter_count;
}

int main() {
  uint8_t transmitter_count;
  uint8_t buffer[3];

  transmitter_count = init();

  for (;;) {
    for (uint8_t current_transmitter = 1; current_transmitter <= transmitter_count; current_transmitter += 1) {
      buffer[0] = POLL_COMMAND;
      buffer[1] = ~POLL_COMMAND;
      rf12_txdata(current_transmitter, buffer, 2);
      if (rf12_rxdata_timeout(buffer, 3)) {
	if (buffer[0] == current_transmitter && buffer[2] == (buffer[0] ^ buffer[1])) {
	  feedback = buffer[1];
	  if (bit_is_set(PORT_LED, PIN_LED)) {
	    LED_OFF();
	  } else {
	    LED_ON();
	  }
	}
      }
    }
    _delay_ms(50);
  }
}
