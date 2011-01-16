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
#include <util/setbaud.h>

#define PIN_S88_RESET                  PB3
#define PIN_S88_LOAD                   PB4
#define PIN_S88_DATA_IN                PB5
#define PIN_S88_DATA_OUT               PB6
#define PIN_S88_CLOCK                  PB7

#define S88_INPUT_BUFFER_SIZE          2
#define BUFFER_SIZE                    24

#define XBEE_FRAME_DELIMITER           0x7E
#define XBEE_FRAME_TYPE_IO_DATA_SAMPLE 0x92

#define STATE_DELIMITER                0
#define STATE_LENGTH_MSB               1
#define STATE_LENGTH_LSB               2
#define STATE_PAYLOAD                  3
#define STATE_CHECKSUM                 4

#define XBEE_FRAME_TYPE(buffer)           buffer[0]
#define XBEE_NUMBER_OF_SAMPLES(buffer)    buffer[12]
#define XBEE_DIGITAL_CHANNEL_MASK(buffer) buffer[14]
#define XBEE_DIGITAL_SAMPLE(buffer)       buffer[17]

volatile uint8_t latch_xbee;
volatile uint8_t s88_input_buffer_head = 0;

static void process_frame();

ISR(USART_RX_vect) {
  static uint8_t state = STATE_DELIMITER, buffer[BUFFER_SIZE], head, checksum;
  static uint16_t length;

  uint8_t c = UDR;

  switch(state) {
  case STATE_DELIMITER:
    if (c == XBEE_FRAME_DELIMITER) {
      head = 0;
      checksum = 0;
      state = STATE_LENGTH_MSB;
    }
    break;
  case STATE_LENGTH_MSB:
    length = c << 8;
    state = STATE_LENGTH_LSB;
    break;
  case STATE_LENGTH_LSB:
    length += c;
    if (length > BUFFER_SIZE) {
      state = STATE_DELIMITER;
    } else {
      state = STATE_PAYLOAD;
    }
    break;
  case STATE_PAYLOAD:
    buffer[head] = c;
    head += 1;
    checksum += c;
    if (head == length) {
      state = STATE_CHECKSUM;
    }
    break;
  case STATE_CHECKSUM:
    if (c == 0xFF - checksum) {
      process_frame(buffer);
    }
    state = STATE_DELIMITER;
    break;
  }
}

ISR(USI_OVERFLOW_vect) {
  static uint8_t s88_input_buffer[S88_INPUT_BUFFER_SIZE];

  s88_input_buffer[s88_input_buffer_head] = USIDR;
  s88_input_buffer_head += 1;
  if (s88_input_buffer_head == S88_INPUT_BUFFER_SIZE) {
    s88_input_buffer_head = 0;
  }
  USIDR = s88_input_buffer[s88_input_buffer_head];
  USISR |= _BV(USIOIF);
}

ISR(PCINT_vect) {
  if (bit_is_set(PINB, PIN_S88_LOAD)) {
    USIDR = latch_xbee;
    USISR = 0x00;
    s88_input_buffer_head = 0;
  }
}

static void process_frame(uint8_t buffer[]) {
  if (XBEE_FRAME_TYPE(buffer) == XBEE_FRAME_TYPE_IO_DATA_SAMPLE) {
    if (XBEE_NUMBER_OF_SAMPLES(buffer) == 1) {
      latch_xbee = ~XBEE_DIGITAL_SAMPLE(buffer) & XBEE_DIGITAL_CHANNEL_MASK(buffer);
    }
  }
}

static void init() {
  DDRB = _BV(PIN_S88_DATA_OUT);

  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRB = _BV(RXEN) | _BV(RXCIE);
  UCSRC = _BV(UCSZ1) | _BV(UCSZ0);

  USICR = _BV(USIOIE) | _BV(USIWM0) | _BV(USICS1) | _BV(USICS0);

  PCMSK = _BV(PCINT4);
  GIMSK |= _BV(PCIE);

  sei();
}

int main() {
  init();  

  for(;;);
}
