#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "nrf24l01.h"
#include "pinDefines.h"
#define F_CPU 1000000UL
#include <util/delay.h>
#include "nokia5110.h"
#include "nrf24l01-mnemonics.h"

void setup_timer(void);
void blink(void);
nRF24L01 *setup_rf(void);

volatile bool rf_interrupt = false;
volatile bool send_message = false;

static inline void initADC0(void) {
  ADMUX |= (1 << REFS0);
  ADCSRA |= (1 << ADPS1);
  ADCSRA |= (1 << ADPS0);
  ADCSRA |= (1 << ADEN);
}

int sensorMeasurement(void);

int main(void) {

  initADC0();
  uint8_t to_address[5] = { 0x03, 0x01, 0x01, 0x01, 0x01 };
  bool on = false;
  sei();
  nRF24L01 *rf = setup_rf();

  // set channel to 120 to avoid interference
  uint8_t data = 0b01111000;
  nRF24L01_write_register(rf, RF_CH, &data, 1);

  setup_timer();
  DDRB &= ~(0);
  DDRD |= (1 << 7);
  PORTD |= (1 << 7);
  LCD_init(0x4c);
  LCD_clear();
  _delay_ms(1000);
  LCD_scroll(8);
  LCD_print(0,40,"...transmitter",0);
  LCD_update();

  while (true) {
      if (rf_interrupt) {
          rf_interrupt = false;
          int success = nRF24L01_transmit_success(rf);
          if (success != 0)
              nRF24L01_flush_transmit_message(rf);
      }

      if (send_message) {
          send_message = false;
          on = !on;
          nRF24L01Message msg;
          if (bit_is_clear(PINB, PB0)) {
            blink();
            memcpy(msg.data, "hej", 3);
          } 
          else {
            LCD_scroll(8);
            LCD_print(0,40,"Sending",0);
            LCD_update();
            int id = 4;
            char src [20];
            char str[10];
            int value = sensorMeasurement();;
            sprintf(str, "%d", value);
            strcpy(src,  str);
            strcat(src, " ");
            sprintf(str, "%d", id);
            strcat(src, str);

            LCD_scroll(8);
            LCD_print(0,40,src,0);
            LCD_update();
            memcpy(msg.data, src, (int)strlen(src));
          }
          msg.length = strlen((char *)msg.data) + 1;
          nRF24L01_transmit(rf, to_address, &msg);
      }
  }

  return 0;
}

void blink(void) {
  for (int i = 0; i < 5; i++) {
    PORTD ^= (1 << 7);
    _delay_ms(100);
  }
  PORTD &= ~(1 << 7);
}

int sensorMeasurement(void) {
  int val = 0;
  ADCSRA |= (1 << ADSC); // start ADC conversion
  loop_until_bit_is_clear(ADCSRA, ADSC); // wait until done
  val = ADC; // read ADC value
  _delay_ms(1000);
  return val;
}

nRF24L01 *setup_rf(void) {
    nRF24L01 *rf = nRF24L01_init();
    rf->ss.port = &PORTB;
    rf->ss.pin = PB2;
    rf->ce.port = &PORTB;
    rf->ce.pin = PB1;
    rf->sck.port = &PORTB;
    rf->sck.pin = PB5;
    rf->mosi.port = &PORTB;
    rf->mosi.pin = PB3;
    rf->miso.port = &PORTB;
    rf->miso.pin = PB4;
    // interrupt on falling edge of INT0 (PD2)
    EICRA |= _BV(ISC01);
    EIMSK |= _BV(INT0);
    nRF24L01_begin(rf);
    return rf;
}

// setup timer to trigger interrupt every second when at 1MHz
void setup_timer(void) {
    TCCR1B |= _BV(WGM12);
    TIMSK1 |= _BV(OCIE1A);
    OCR1A = 15624;
    TCCR1B |= _BV(CS10) | _BV(CS11);
}

// each one second interrupt
ISR(TIMER1_COMPA_vect) {
   send_message = true;
}

// nRF24L01 interrupt
ISR(INT0_vect) {
    rf_interrupt = true;
}






// Code for the DHT11 sensor data
#define DHT11_PIN 6
uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;

void Request()						/* Microcontroller send start pulse or request */
{
	DDRD |= (1<<DHT11_PIN);
	PORTD &= ~(1<<DHT11_PIN);		/* set to low pin */
	_delay_ms(20);					/* wait for 20ms */
	PORTD |= (1<<DHT11_PIN);		/* set to high pin */
}

void Response()						/* receive response from DHT11 */
{
	DDRD &= ~(1<<DHT11_PIN);
	while(PIND & (1<<DHT11_PIN));
	while((PIND & (1<<DHT11_PIN))==0);
	while(PIND & (1<<DHT11_PIN));
}

uint8_t Receive_data()							/* receive data */
{
	for (int q=0; q<8; q++)
	{
		while((PIND & (1<<DHT11_PIN)) == 0);	/* check received bit 0 or 1 */
		_delay_us(30);
		if(PIND & (1<<DHT11_PIN))				/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);						/* then its logic HIGH */
		else									/* otherwise its logic LOW */
		c = (c<<1);
		while(PIND & (1<<DHT11_PIN));
	}
	return c;
}


int Getsensordata() {

	Request();				/* send start pulse */

	Response();				/* receive response */

	I_RH=Receive_data();	/* store first eight bit in I_RH */

	D_RH=Receive_data();	/* store next eight bit in D_RH */
	I_Temp=Receive_data();	/* store next eight bit in I_Temp */
	D_Temp=Receive_data();	/* store next eight bit in D_Temp */
	CheckSum=Receive_data();/* store next eight bit in CheckSum */
	_delay_ms(1000); 
	
	return I_Temp; 
}