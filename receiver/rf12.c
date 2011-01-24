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
#include <util/setbaud.h>
#include "rf12.h"

#define PIN_SEL PD3
#define PIN_SDI PD1
#define PIN_SDO PD0
#define PIN_SCK PD4
#define PIN_IRQ PD2

#define NODE_ID 0x00

#define USICR_CLOCK       _BV(USIWM0) | _BV(USITC)
#define USICR_SHIFT_CLOCK _BV(USIWM0) | _BV(USITC) | _BV(USICLK)

#define CHIP_SELECT_ON()  PORTB &= ~_BV(PIN_SEL)
#define CHIP_SELECT_OFF() PORTB |= _BV(PIN_SEL)

uint16_t rf12_trans(uint16_t value) {
  uint16_t result;

  CHIP_SELECT_ON();  
  UDR0 = value >> 8;
  loop_until_bit_is_set(UCSR0A, TXC0);
  result = UDR0 << 8;
  UCSR0A |= _BV(TXC0);
  UDR0 = value;
  loop_until_bit_is_set(UCSR0A, TXC0);
  result |= UDR0;
  UCSR0A |= _BV(TXC0);
  CHIP_SELECT_OFF();

  return result;
}

void rf12_ready(void) {
  loop_until_bit_is_clear(PIND, PIN_IRQ);
  rf12_trans(0x0000);
}

void rf12_init() {
  UBRR0 = 0;
  PORTD |= _BV(PIN_SEL) | _BV(PIN_IRQ);
  DDRD |= _BV(PIN_SCK) | _BV(PIN_SEL) | _BV(PIN_SDO);
  
  UCSR0C |= _BV(UMSEL01) | _BV(UMSEL00);
  UCSR0B |= _BV(TXEN0);
  UBRR0 = 0;

  rf12_trans(0x0000);
  rf12_trans(0x8205);
  rf12_trans(0xB800);
  while (bit_is_clear(PIND, PIN_IRQ)) {
    rf12_trans(0x0000);
  }
  PORTC |= _BV(PC1);

  rf12_trans(0x8201);
  rf12_trans(0x80E7);            // enable TX register, enable RX FIFO buffer, 868MHz, 12.0pF
  rf12_trans(0xA460);            // 868.000MHz
  rf12_trans(0xC606);            // 49.26kbps
  rf12_trans(0x94A2);            // VDI,FAST,134kHz,0dBm,-91dBm
  rf12_trans(0xC2AC);            // AL,!ml,DIG,DQD4
  rf12_trans(0xCA83);            // FIFO8,SYNC,!ff,DR
  rf12_trans(0xCE00 | NODE_ID);  // SYNC=2DD4
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
  PORTC |= _BV(PC0);
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
  rf12_trans(0x8209);			// TX off
  rf12_ready();
  rf12_trans(0xB8AA);
}

void rf12_rxdata(uint8_t *data, uint8_t number) {
  rf12_trans(0x82D9);			// RX on
  for (uint8_t i = 0; i < number; i++) {
    rf12_ready();
    *data++ = rf12_trans(0xB000);
  }
  rf12_trans(0x8209);			// RX off
}
