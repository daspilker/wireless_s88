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
#include "stdbool.h"
#include "rf12.h"

#define PIN_SDI PD0
#define PIN_SDO PD1
#define PIN_IRQ PD2
#define PIN_SEL PD3
#define PIN_SCK PD4

#define CHIP_SELECT_ON()  PORTD &= ~_BV(PIN_SEL)
#define CHIP_SELECT_OFF() PORTD |= _BV(PIN_SEL)

static uint8_t rf12_byte(uint8_t value) {
  UDR0 = value;
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

static uint16_t rf12_trans(uint16_t value) {
  uint16_t result;

  CHIP_SELECT_ON();  
  result = rf12_byte(value >> 8) << 8;
  result |= rf12_byte(value);
  CHIP_SELECT_OFF();

  return result;
}

static void rf12_ready(void) {
  loop_until_bit_is_clear(PIND, PIN_IRQ);
  rf12_trans(0x0000);
}

static bool rf12_ready_timeout(void) {
  TCNT0 = 0x00;
  TIFR0 |= _BV(TOV0);
  TCCR0B = _BV(CS02) | _BV(CS00);

  while(bit_is_set(PIND, PIN_IRQ) && bit_is_clear(TIFR0, TOV0));

  TCCR0B = 0x00;
  rf12_trans(0x0000);
  if (bit_is_set(TIFR0, TOV0)) {
    return false;
  } else {
    return true;
  }
}

void rf12_init(uint8_t node_id) {
  PORTD |= _BV(PIN_SEL) | _BV(PIN_IRQ);
  DDRD |= _BV(PIN_SCK) | _BV(PIN_SEL) | _BV(PIN_SDO);
  
  UCSR0C = _BV(UMSEL01) | _BV(UMSEL00);
  UCSR0B = _BV(TXEN0) | _BV(RXEN0);
  UBRR0 = 1;

  rf12_trans(0x0000);
  rf12_trans(0x8201);
  rf12_trans(0xB800);
  while (bit_is_clear(PIND, PIN_IRQ)) {
    rf12_trans(0x0000);
  }

  rf12_trans(0x80E7);            // enable TX register, enable RX FIFO buffer, 868MHz, 12.0pF
  rf12_trans(0xA460);            // 868.000MHz
  rf12_trans(0xC606);            // 49.26kbps
  rf12_trans(0x94A2);            // VDI,FAST,134kHz,0dBm,-91dBm
  rf12_trans(0xC2AC);            // AL,!ml,DIG,DQD4
  rf12_trans(0xCA83);            // FIFO8,SYNC,!ff,DR
  rf12_trans(0xCE00 | node_id);  // SYNC=2DD4
  rf12_trans(0xC483);            // @PWR,NO RSTRIC,!st,!fi,OE,EN
  rf12_trans(0x9850);            // !mp,90kHz,MAX OUT
  rf12_trans(0xCC77);            // OB1，OB0, LPX,！ddy，DDIT，BW0
  rf12_trans(0xE000);            // NOT USE
  rf12_trans(0xC800);            // NOT USE
  rf12_trans(0xC049);            // 1.66MHz,3.1V
}

void rf12_txdata(uint8_t node_id, uint8_t *data, uint8_t number) {
  rf12_trans(0x8239);			// TX on
  rf12_ready();
  rf12_trans(0xB8AA);
  rf12_ready();
  rf12_trans(0xB8AA);
  rf12_ready();
  rf12_trans(0xB8AA);
  rf12_ready();
  rf12_trans(0xB82D);
  rf12_ready();
  rf12_trans(0xB800 | node_id);
  for (uint8_t i = 0; i < number; i++) {
    rf12_ready();
    rf12_trans(0xB800 | (*data++));
  }
  rf12_ready();
  rf12_trans(0xB8AA);
  rf12_ready();
  rf12_trans(0x8201);			// TX off
  rf12_ready();
  rf12_trans(0xB8AA);
}

void rf12_rxdata(uint8_t *data, uint8_t number) {
  rf12_trans(0x82D9);			// RX on
  for (uint8_t i = 0; i < number; i++) {
    rf12_ready();
    *data++ = rf12_trans(0xB000);
  }
  rf12_trans(0x8201);			// RX off
}

bool rf12_rxdata_timeout(uint8_t *data, uint8_t number) {
  rf12_trans(0x82D9);			// RX on
  for (uint8_t i = 0; i < number; i++) {
    if (!rf12_ready_timeout()) {
      rf12_trans(0x8201);			// RX off
      return false;
    }
    *data++ = rf12_trans(0xB000);
  }
  rf12_trans(0x8201);			// RX off
  return true;
}
