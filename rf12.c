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
#include "rf12.h"

#define PIN_SEL PB4
#define PIN_SDI PB5
#define PIN_SDO PB6
#define PIN_SCK PB7
#define PIN_IRQ PD2

#define USICR_CLOCK       _BV(USIWM0) | _BV(USITC)
#define USICR_SHIFT_CLOCK _BV(USIWM0) | _BV(USITC) | _BV(USICLK)

#define CHIP_SELECT_ON()  PORTB &= ~_BV(PIN_SEL)
#define CHIP_SELECT_OFF() PORTB |= _BV(PIN_SEL)

#define USIBR _SFR_IO8(0x000)

uint16_t rf12_trans(uint16_t value) {
  uint16_t result;

  CHIP_SELECT_ON();
  USIDR = value >> 8;
  for (uint8_t i=0; i<8; i++) {
    USICR = USICR_CLOCK;
    USICR = USICR_SHIFT_CLOCK;
  }
  //  result = USIBR << 8;
  USIDR = value;
  for (uint8_t i=0; i<8; i++) {
    USICR = USICR_CLOCK;
    USICR = USICR_SHIFT_CLOCK;
  }
  //  result |= USIBR;
  CHIP_SELECT_OFF();

  return result;
}

void rf12_ready(void) {
  CHIP_SELECT_ON();
  loop_until_bit_is_clear(PIND, PIN_IRQ);
}

void rf12_init(void) {
  DDRB  |= _BV(PIN_SDO) | _BV(PIN_SCK) | _BV(PIN_SEL);
  PORTB |= _BV(PIN_SEL);
  PORTD |= _BV(PIN_IRQ);

  rf12_trans(0x0000);
  while (bit_is_clear(PIND, PIN_IRQ)) {
    rf12_trans(0x0000);
  }
  
  rf12_trans(0xC0E0);	                  // AVR CLK: 10MHz
  rf12_trans(0x80E7);			  // Enable FIFO, 868MHz
  rf12_trans(0xC2AB);			  // Data Filter: internal
  rf12_trans(0xCA81);			  // Set FIFO mode
  rf12_trans(0xE000);			  // disable wakeuptimer
  rf12_trans(0xC800);			  // disable low duty cycle
  rf12_trans(0xC4F7);			  // AFC settings: autotuning: -10kHz...+7,5kHz
  rf12_trans(0xA620);                     // Sende/Empfangsfrequenz auf 433,92MHz einstellen
  rf12_trans(0x948C);                     // 200kHz Bandbreite, -6dB Verstärkung, DRSSI threshold: -79dBm 
  rf12_trans(0xC610);                     // 19200 baud
  rf12_trans(0x9860);                     // 1mW Ausgangangsleistung, 120kHz Frequenzshift
}

void rf12_txdata(unsigned char *data, unsigned char number) {
  rf12_trans(0x8238);			// TX on
  rf12_ready();
  rf12_trans(0xB8AA);
  rf12_ready();
  rf12_trans(0xB8AA);
  rf12_ready();
  rf12_trans(0xB8AA);
  rf12_ready();
  rf12_trans(0xB82D);
  rf12_ready();
  rf12_trans(0xB8D4);
  for (uint8_t i = 0; i < number; i++) {
    rf12_ready();
    rf12_trans(0xB800 | (*data++));
  }
  rf12_ready();
  rf12_trans(0x8208);			// TX off
}

void rf12_rxdata(unsigned char *data, unsigned char number) {
  rf12_trans(0x82C8);			// RX on
  rf12_trans(0xCA81);			// set FIFO mode
  rf12_trans(0xCA83);			// enable FIFO
  for (uint8_t i = 0; i < number; i++) {
    rf12_ready();
    *data++ = rf12_trans(0xB000);
  }
  rf12_trans(0x8208);			// RX off
}
