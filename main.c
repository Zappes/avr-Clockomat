#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lib/clockdisplay.h"

/*
 * This one doesn't do much. It just calls the two other functions.
 */
int main(void) {
	clock_init();

	// Enable Global Interrupts
	sei();

	while(1) {
		for (uint8_t h = 0; h < 24; h++) {
			for (uint8_t m = 0; m < 60; m++) {
				putTime(h, m);
				_delay_ms(1000);
			}
		}
	}

	return 0;
}

