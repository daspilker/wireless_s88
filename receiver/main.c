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

#define BUFFER_SIZE 128

volatile uint8_t feedback[BUFFER_SIZE];
volatile uint8_t feedback_out = 0;
volatile uint8_t input[BUFFER_SIZE];
volatile uint8_t input_in = 0;
volatile uint8_t input_out = 0;

volatile uint8_t transmitter_count = 0;
volatile uint8_t s88_bytes = 0;

ISR(SPI_STC_vect) {
  if (feedback_out == s88_bytes) {
    SPDR = input[input_out];
    input_out += 1;
  } else {
    SPDR = feedback[feedback_out];
    feedback_out += 1;
  }
  input[input_in] = SPDR;
  input_in += 1;
  if (input_in == BUFFER_SIZE) {
    input_in = 0;
  }
  if (input_out == BUFFER_SIZE) {
    input_out = 0;
  }
}

ISR(PCINT0_vect) {
  if (bit_is_set(PINB, PIN_S88LOAD)) {
    SPCR &= ~_BV(SPE);
    SPCR |= _BV(SPE);
    SPDR = feedback[0];
    feedback_out = 1;
    input_in = 0;
    input_out = 0;
  }
}

static void set_feedback(uint8_t transmitter, uint8_t value) {
  bool low_nibble = transmitter & 0x01;
  uint8_t feedback_position = transmitter >> 1;
  feedback[feedback_position] = (feedback[feedback_position] & (low_nibble ? 0xF0 : 0x0F)) + (value << (low_nibble ? 0 : 4));
}

static void init() {
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
  s88_bytes = (((transmitter_count - 1) >> 2) + 1) << 1;

  rf12_init(NODE_ID);

  sei();
}

int main() {
  uint8_t buffer[3];

  init();

  for (;;) {
    for (uint8_t current_transmitter = 0; current_transmitter < transmitter_count; current_transmitter += 1) {
      buffer[0] = POLL_COMMAND;
      buffer[1] = ~POLL_COMMAND;
      rf12_txdata(current_transmitter + 1, buffer, 2);
      if (rf12_rxdata_timeout(buffer, 3)) {
	if (buffer[0] == current_transmitter + 1 && buffer[2] == (buffer[0] ^ buffer[1])) {
	  set_feedback(current_transmitter, buffer[1] & 0x0F);
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
