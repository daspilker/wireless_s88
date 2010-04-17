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

#define PIN_LED       PD5
#define PIN_RFM12_SEL PB4
#define PIN_RFM12_SDI PB5
#define PIN_RFM12_SDO PB6
#define PIN_RFM12_SCK PB7
#define PIN_RFM12_IRQ PD2

#define LED_ON()  PORTD |=  _BV(PIN_LED)
#define LED_OFF() PORTD &= ~_BV(PIN_LED)

#define SEL_OUTPUT()  DDRB   |=  _BV(PIN_RFM12_SEL)
#define SEL_HI()      PORTB  |=  _BV(PIN_RFM12_SEL)
#define SEL_LOW()     PORTB  &= ~_BV(PIN_RFM12_SEL)
#define SDI_OUTPUT()  DDRB   |=  _BV(PIN_RFM12_SDI)
#define SDI_HI()      PORTB  |=  _BV(PIN_RFM12_SDI)
#define SDI_LOW()     PORTB  &= ~_BV(PIN_RFM12_SDI)
#define SCK_OUTPUT()  DDRB   |=  _BV(PIN_RFM12_SCK)
#define SCK_HI()      PORTB  |=  _BV(PIN_RFM12_SCK)
#define SCK_LOW()     PORTB  &= ~_BV(PIN_RFM12_SCK)

#define SDO_HI()      PINB   &   _BV(PIN_RFM12_SDO)
#define IRQ_HI()      PIND   &   _BV(PIN_RFM12_IRQ)

#define PORT_SEL  PORTB
#define DDR_SEL   DDRB
#define PORT_SDI  PORTB
#define DDR_SDI   DDRB
#define PORT_SCK  PORTB
#define DDR_SCK   DDRB
#define PORT_SDO  PORTB
#define DDR_SDO   DDRB

void RFXX_PORT_INIT(void) {
  SEL_HI();
  SDI_HI();
  SEL_OUTPUT();
  SDI_OUTPUT();
  SCK_OUTPUT();
}

uint16_t RFXX_WRT_CMD(uint16_t cmd) {
  uint16_t temp = 0;

  SCK_LOW();
  SEL_LOW();
  for (uint8_t i=0; i<16; i++) {
    temp <<= 1;
    if (SDO_HI()) {
      temp |= 0x0001;
    }
    SCK_LOW();
    if (cmd & 0x8000) {
      SDI_HI();
    } else {
      SDI_LOW();
    }
    SCK_HI();
    cmd <<= 1;
  }
  SCK_LOW();
  SEL_HI();
  return temp;
}

void RF12_INIT(void){
  RFXX_WRT_CMD(0x80E8); // EL,EF,868band,12.5pF
  RFXX_WRT_CMD(0x8239); // !er,!ebb,ET,ES,EX,!eb,!ew,DC
  RFXX_WRT_CMD(0xA640); // A140=430.8MHz
  RFXX_WRT_CMD(0xC647); // 4.8kbps
  RFXX_WRT_CMD(0x94A0); // VDI,FAST,134kHz,0dBm,-103dBm
  RFXX_WRT_CMD(0xC2AC); // AL,!ml,DIG,DQD4
  RFXX_WRT_CMD(0xCA81); // FIFO8,SYNC,!ff,DR
  RFXX_WRT_CMD(0xCED4); // SYNC=2DD4 
  RFXX_WRT_CMD(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN
  RFXX_WRT_CMD(0x9850); // !mp,9810=30kHz,MAX OUT
  RFXX_WRT_CMD(0xCC77); // OB1 OB0, lpx, ddy DDIT BW0
  RFXX_WRT_CMD(0xE000); // NOT USE
  RFXX_WRT_CMD(0xC800); // NOT USE
  RFXX_WRT_CMD(0xC040); // 1.66MHz,2.2V
}

void RF12_SEND(uint8_t byte) {
  while (IRQ_HI()); // wait for previously TX over
  RFXX_WRT_CMD(0xB800 + byte);
}

int main(void) {
  uint8_t checksum;

  cli();

  DDRD |= _BV(PIN_LED);

  for (uint8_t i=0; i<3; i++) {
    _delay_ms(200);
    LED_ON();
    _delay_ms(200);
    LED_OFF();
  }

  RFXX_PORT_INIT();
  RF12_INIT();

  while (1) {
    LED_ON();
    RFXX_WRT_CMD(0x0000); // read status register
    RFXX_WRT_CMD(0x8239); // !er,!ebb,ET,ES,EX,!eb,!ew,DC
    checksum = 0;
    RF12_SEND(0xAA);      //PREAMBLE
    RF12_SEND(0xAA);      //PREAMBLE
    RF12_SEND(0xAA);      //PREAMBLE
    RF12_SEND(0x2D);      //SYNC HI BYTE
    RF12_SEND(0xD4);      //SYNC LOW BYTE
    RF12_SEND(0x30);      //DATA BYTE 0
    checksum += 0x30;
    RF12_SEND(0x31);      //DATA BYTE 1
    checksum += 0x31;
    RF12_SEND(0x32);
    checksum += 0x32;
    RF12_SEND(0x33);
    checksum += 0x33;
    RF12_SEND(0x34);
    checksum += 0x34;
    RF12_SEND(0x35);
    checksum += 0x35;
    RF12_SEND(0x36);
    checksum += 0x36;
    RF12_SEND(0x37);
    checksum += 0x37;
    RF12_SEND(0x38);
    checksum += 0x38;
    RF12_SEND(0x39);
    checksum += 0x39;
    RF12_SEND(0x3A);
    checksum += 0x3A;
    RF12_SEND(0x3B);
    checksum += 0x3B;
    RF12_SEND(0x3C);
    checksum += 0x3C;
    RF12_SEND(0x3D);
    checksum += 0x3D;
    RF12_SEND(0x3E);
    checksum += 0x3E;
    RF12_SEND(0x3F);      // DATA BYTE 15
    checksum += 0x3F;
    RF12_SEND(checksum);  // send chek sum
    RF12_SEND(0xAA);      // DUMMY BYTE
    RF12_SEND(0xAA);      // DUMMY BYTE
    RF12_SEND(0xAA);      // DUMMY BYTE
    RFXX_WRT_CMD(0x8201);
    LED_OFF();
    _delay_ms(1000);
  }
  return 0;
}
