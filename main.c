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

#define S88_LOAD                       bit_is_set(PINB, PIN_S88_LOAD)

#define BUFFER_SIZE                    24

#define XBEE_FRAME_DELIMITER           0x7E
#define XBEE_FRAME_TYPE_IO_DATA_SAMPLE 0x92

#define STATE_DELIMITER                0
#define STATE_LENGTH_MSB               1
#define STATE_LENGTH_LSB               2
#define STATE_PAYLOAD                  3
#define STATE_CHECKSUM                 4

volatile uint8_t latch_xbee;

static void process_frame();
static void init();

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
    head++;
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
  static uint8_t data_temp, counter;

  USISR |= 0x0E;

  if (S88_LOAD) {
    USIDR = latch_xbee;
    data_temp = 0;
    counter = 0;
  } else {
    counter++;
    if (counter == 8) {
      uint8_t data_in = USIDR;
      USIDR = data_temp;
      data_temp = data_in;
      counter = 0;
    }
  }
}

static void process_frame(uint8_t buffer[]) {
  if (buffer[0] == XBEE_FRAME_TYPE_IO_DATA_SAMPLE) {
    if (buffer[12] == 0x01) {
      latch_xbee = ~buffer[17] & buffer[14];
    }
  }
}

static void init() {
  DDRB = _BV(PIN_S88_DATA_OUT);
  PORTB = _BV(PIN_S88_LOAD) | _BV(PIN_S88_CLOCK);

  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRB = _BV(RXEN) | _BV(RXCIE);
  UCSRC = _BV(UCSZ1) | _BV(UCSZ0);

  loop_until_bit_is_clear(PINB, PIN_S88_CLOCK);

  USICR = _BV(USIOIE) | _BV(USIWM0) | _BV(USICS1);
  USISR |= 0x0F;

  sei();
}

int main() {
  init();  

  for(;;);
}
