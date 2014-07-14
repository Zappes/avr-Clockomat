#include <avr/interrupt.h>

#include "clockdisplay.h"
#include "tlc5940.h"

#include "clockcontrol.h"
#include "alarm.h"

uint8_t current_color = 0;
uint8_t anim_direction = ANIMATION_DIR_NONE;

uint16_t linebuffer[6];

uint16_t numbers[] = {
0b0111101101101111,
0b0001001001001001,
0b0111001111100111,
0b0111001111001111,
0b0101101111001001,
0b0111100111001111,
0b0111100111101111,
0b0111001001001001,
0b0111101111101111,
0b0111101111001111
};

volatile uint16_t color_fg[] = {
		0,
		0,
		0,
		0,
		0,
		0
};

volatile uint16_t color_bg[] = {
		0,
		0,
		0,
		0,
		0,
		0
};

volatile uint16_t switch_off_counter = 0;

ISR(INT0_vect)
{
	alarm_stop_sound();
	start_fade_in();
	switch_off_counter = 20000;
}

void display_time() {
	put_time_bcd(time_get_hours_bcd(), time_get_minutes_bcd());
}

void display_alarm() {
	put_time(alarm_get_hours(), alarm_get_minutes());
}


void set_color(uint8_t color) {
	color &= 63;
	uint16_t gs = color * color;

	for(uint8_t row = 0; row < 6; row++) {
		color_fg[row] = gs;
	}
}

void display_init() {
	// initialize the LED controller
	TLC5940_Init();

	// Default all channels to off
	TLC5940_SetAllGS(0);

	// set foreground color to 0
	set_color(0);

	// enable int0 on pd2
  DDRD &= ~_BV(PD2);
  EICRA |= _BV(ISC00) | _BV(ISC01);
  EIMSK |= _BV(INT0);
 }

void put_number(uint8_t numIndex, uint8_t slot) {
	uint8_t slotStartBit = (slot << 2);

	for(uint8_t line = 0; line < 5; line++) {
		for(uint8_t numBit = 0; numBit < 3; numBit++) {
			uint8_t numBitPos = (12 + numBit) - (line << 1) - line;

			if(numbers[numIndex] & (1 << numBitPos)) {
				linebuffer[line] |= (1 << (slotStartBit + numBit));
			}	else {
				linebuffer[line] &= ~(1 << (slotStartBit + numBit));
			}
		}
	}
}

void put_time(uint8_t hours, uint8_t minutes) {
	put_number(minutes % 10, 0);
	put_number(minutes / 10, 1);
	put_number(hours % 10, 2);
	put_number(hours / 10, 3);
}

void put_time_bcd(uint8_t hours, uint8_t minutes) {
	put_number(minutes & 0x0F, 0);
	put_number((minutes & 0xF0) >> 4, 1);
	put_number(hours & 0x0F, 2);
	put_number((hours & 0xF0) >> 4, 3);
}

void put_status(uint16_t status) {
	linebuffer[5] = status;
}

void set_statusbit(uint8_t bit) {
	linebuffer[5] |= _BV(bit);
}

void clear_statusbit(uint8_t bit) {
	linebuffer[5] &= ~_BV(bit);
}

void paint_line(uint8_t line) {
	for(uint8_t i = 0; i < 16; i++) {
		TLC5940_SetGS(i, (linebuffer[line] & (1 << i)) ? color_fg[line] : color_bg[line]);
	}
}

void start_fade_in() {
	anim_direction = ANIMATION_DIR_IN;
}

void start_fade_out() {
	anim_direction = ANIMATION_DIR_OUT;
}

void keep_display_on() {
	anim_direction = ANIMATION_DIR_IN;
	switch_off_counter = 20000;
}

void animate() {
	switch(anim_direction) {
		case ANIMATION_DIR_IN:
			if(current_color >= 62) {
				anim_direction = ANIMATION_DIR_NONE;
				set_color(63);
			}
			else {
				current_color+=2;
				set_color(current_color);
			}

			break;
		case ANIMATION_DIR_OUT:
			if(current_color <= 1) {
				anim_direction = ANIMATION_DIR_NONE;
				set_color(0);
			}
			else {
				current_color-=2;
				set_color(current_color);
			}
			break;
	}
}


