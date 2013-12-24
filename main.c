#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DDR_OUT DDRB
#define PORT_OUT PORTB
#define PIN_OUT PB2
#define DDR_IN DDRB
#define PORT_IN PINB
#define PIN_IN PB1

int main(void) {
	CLKPR = 0b10000000;
	CLKPR = 0b00000000;
	// Output:
	DDR_OUT |= (1 << PIN_OUT);
	// Input:
	DDR_IN &= ~(1 << PIN_IN);
	DDR_IN &= ~(1 << PB0);

	// Timer 0 configuration
	TCCR0B =  (0<<CS01) | (1<<CS00); // no prescaler
	TIMSK |= (1<<TOIE0);

	PORT_OUT &= ~(1 << PIN_OUT);
	sei();

	while (1) { }

	return 0;
}

volatile uint16_t buffer = 0;
volatile uint8_t ones_count = 0;
volatile uint8_t state = 0;

volatile int16_t time = 0;
volatile int16_t lastTimings[16] ; 

ISR (TIM0_OVF_vect)
{
	time++;
	//shift buffer:
	if((buffer >> 15) & 1) {
		//shifts a 1 away:
		ones_count--;
	}
	buffer <<= 1;

	//add 1 to buffer:
	if((PORT_IN & (1 << PIN_IN))) {
		buffer |= 1;
		ones_count++;
	}

	if(state) {
		if(ones_count <= 8) {
			for(uint8_t i = 0; i < 15; i++) {
				lastTimings[i] = lastTimings[i+1];
			}
			lastTimings[15] = time;
			PORT_OUT &= ~(1 << PIN_OUT);
			state = 0;
		}
	} else {
		if(ones_count >= 10) {
			uint8_t count = 0;
			for(uint8_t i = 0; i < 15; i++) {
				if(lastTimings[i] > 15 && lastTimings[i] < 40) {
					count++;
				}
			}
			if(count >= 13) PORT_OUT |= (1 << PIN_OUT);
			state = 1;
			time = 0;
		}
	}

}