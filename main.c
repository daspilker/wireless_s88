//
// Copyright (c) Daniel A. Spilker.
// All rights reserved.
//
// This work is licensed under the Creative Commons Attribution 3.0 Germany License.
// To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/de/.
//
// http://daniel-spilker.com/
//

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define PIN_LED PD6

#define PIN_SEL PB4
#define PIN_SDI PB5
#define PIN_SDO PB6
#define PIN_SCK PB7

#define LED_ON()  PORTD |=  _BV(PIN_LED)
#define LED_OFF() PORTD &= ~_BV(PIN_LED)

#define RF12FREQ(freq)	((freq-430.0)/0.0025)							// macro for calculating frequency value out of frequency in MHz

#define USICR_CLOCK       _BV(USIWM0) | _BV(USITC)
#define USICR_SHIFT_CLOCK _BV(USIWM0) | _BV(USITC) | _BV(USICLK)

#define CHIP_SELECT_ON()  PORTB &= ~_BV(PIN_SEL)
#define CHIP_SELECT_OFF() PORTB |= _BV(PIN_SEL)

static void rf12_trans(uint16_t value) {
  CHIP_SELECT_ON();
  USIDR = value >> 8;
  for (uint8_t i=0; i<8; i++) {
    USICR = USICR_CLOCK;
    USICR = USICR_SHIFT_CLOCK;
  }
  USIDR = value;
  for (uint8_t i=0; i<8; i++) {
    USICR = USICR_CLOCK;
    USICR = USICR_SHIFT_CLOCK;
  }
  CHIP_SELECT_OFF();
}

static void rf12_setbandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi) {
  rf12_trans(0x9400|((bandwidth&7)<<5)|((gain&3)<<3)|(drssi&7));
}

static void rf12_setfreq(unsigned short freq) {
  if (freq<96)				// 430,2400MHz
    freq=96;
  else if (freq>3903)			// 439,7575MHz
    freq=3903;
  rf12_trans(0xA000|freq);
}

static void rf12_setbaud(unsigned short baud) {
  if (baud<663)
    return;
  if (baud<5400)					// Baudrate= 344827,58621/(R+1)/(1+CS*7)
    rf12_trans(0xC680|((43104/baud)-1));
  else
    rf12_trans(0xC600|((344828UL/baud)-1));
}

static void rf12_setpower(unsigned char power, unsigned char mod) {	
  rf12_trans(0x9800|(power&7)|((mod&15)<<4));
}

static void rf12_init(void) {
  DDRB  |= _BV(PIN_SDO) | _BV(PIN_SCK) | _BV(PIN_SEL);
  PORTB |= _BV(PIN_SEL);

  _delay_ms(100);			// wait until POR done
  
  rf12_trans(0xC0E0);			// AVR CLK: 10MHz
  rf12_trans(0x80E7);			// Enable FIFO, 868MHz
  rf12_trans(0xC2AB);			// Data Filter: internal
  rf12_trans(0xCA81);			// Set FIFO mode
  rf12_trans(0xE000);			// disable wakeuptimer
  rf12_trans(0xC800);			// disable low duty cycle
  rf12_trans(0xC4F7);			// AFC settings: autotuning: -10kHz...+7,5kHz

  rf12_setfreq(RF12FREQ(433.92));	// Sende/Empfangsfrequenz auf 433,92MHz einstellen
  rf12_setbandwidth(4, 1, 4);		// 200kHz Bandbreite, -6dB Verstärkung, DRSSI threshold: -79dBm 
  rf12_setbaud(19200);			// 19200 baud
  rf12_setpower(0, 6);			// 1mW Ausgangangsleistung, 120kHz Frequenzshift
}

void rf12_ready(void) {
  CHIP_SELECT_ON();
  loop_until_bit_is_set(PINB, PIN_SDI);
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
  for (uint8_t i=0; i<number; i++) {
    rf12_ready();
    rf12_trans(0xB800|(*data++));
  }
  rf12_ready();
  rf12_trans(0x8208);			// TX off
}

int main(void) {
  DDRD |= _BV(PIN_LED);

  for (uint8_t i=0; i<3; i++) {
    _delay_ms(50);
    LED_ON();
    _delay_ms(50);
    LED_OFF();
  }

  rf12_init();

  uint8_t i = 0;
  unsigned char test[] = "ab";
  for (;;) {
    rf12_txdata(test, 2);
    if (i%2 == 0) {
      LED_ON();
    } else {
      LED_OFF();
    }
    i+=1;
    _delay_ms(200);
  }
}

