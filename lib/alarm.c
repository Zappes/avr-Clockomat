#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "alarm.h"
#include "persistence.h"
#include "clockdisplay.h"
#include "status.h"

uint8_t alarm_minutes	= 0;
uint8_t alarm_hours		= 0;
uint8_t alarm_enabled = 0;

volatile uint8_t alarm_sound_level	= 0;
volatile uint16_t alarm_sound_count	= 0;
volatile uint8_t alarm_sound_started = 0;
volatile uint8_t alarm_current_beep_numer = 0;

void alarm_init() {
	// enable input on the pins for the button and the rotary encoder
	ALARM_DDR &= ~(_BV(ALARM_SHOW_BUTTON) | _BV(ALARM_INC) | _BV(ALARM_DEC));

	// read alarm settings from eeprom, if available
	persistence_restore(&alarm_enabled, &alarm_hours, &alarm_minutes);

	if(alarm_is_enabled()) {
		set_statusbit(STATUS_ALARM);
	}
	else {
		clear_statusbit(STATUS_ALARM);
	}
}

void alarm_enable_setting(uint8_t enable) {
	if(enable) {
		PCICR |= _BV(PCIE2);
		PCMSK2 |= _BV(PCINT21) | _BV(PCINT22);
	}
	else {
		PCICR &= ~_BV(PCIE2);
		PCMSK2 &= ~(_BV(PCINT21) | _BV(PCINT22));
	}
}

ISR(PCINT2_vect) {
	uint8_t val = ALARM_INPORT;

	if(val & _BV(ALARM_INC)) {
		alarm_increment();
	}
	else if(val & _BV(ALARM_DEC)) {
		alarm_decrement();
	}
}

uint8_t alarm_is_shown() {
	return (ALARM_INPORT & _BV(ALARM_SHOW_BUTTON)) != 0;
}

uint8_t alarm_get_hours() {
	return alarm_hours;
}

uint8_t alarm_get_minutes() {
	return alarm_minutes;
}

uint8_t alarm_is_enabled() {
	return alarm_enabled;
}

void alarm_update_enable() {
	static uint8_t previous_state = 0;

	if(ALARM_INPORT & _BV(ALARM_ENABLE_BUTTON)) {
		if(previous_state == 0) {
			alarm_enabled ^= 1;
		}

		previous_state = 1;
	} else {
		previous_state = 0;
	}
}

void alarm_increment() {
	alarm_minutes++;
	if(alarm_minutes > 59) {
		alarm_minutes = 0;
		alarm_hours++;
	}
	if(alarm_hours > 23) {
		alarm_hours = 0;
	}
}

void alarm_decrement() {
	if(alarm_minutes > 0) {
		alarm_minutes--;
	}
	else {
		alarm_minutes = 59;
		if(alarm_hours > 0) {
			alarm_hours--;
		}
		else {
			alarm_hours = 23;
		}
	}
}

uint8_t alarm_is_sound_started() {
	return alarm_sound_started;
}

void alarm_start_sound() {
	// enable output on the pin to which the alarm beeper is connected
	ALARM_DDR |= _BV(ALARM_SOUNDPIN);

	// disable the timer
	TCCR2B = 0;

	// set PWM for 50% duty cycle
	OCR2B = ALARM_BEEP_VOLUME;

	// set none-inverting mode
	TCCR2A |= (1 << COM2B1);

	// set fast PWM Mode
	TCCR2A |= (1 << WGM21) | (1 << WGM20);

	// enable interrupt on overflow
	TIMSK2 |= _BV(TOIE2);

	alarm_sound_level = 1;
	alarm_sound_count = 0;
	alarm_sound_started = 1;
	alarm_current_beep_numer = 0;

	// set prescaler to 8 and starts PWM
	TCCR2B |= (1 << CS21);
}

void alarm_stop_sound() {
	// disable the timer
	TCCR2B = 0;

	alarm_sound_level = 0;
	alarm_sound_count = 0;
	alarm_sound_started = 0;
	alarm_current_beep_numer = 0;

	// switch off signal. if the PWM was just running, this might still be high, otherwise.
	ALARM_OUTPORT &= ~_BV(ALARM_SOUNDPIN);

	// disable output on the pin to which the alarm beeper is connected
	ALARM_DDR &= ~_BV(ALARM_SOUNDPIN);
}

ISR(TIMER2_OVF_vect) {
	alarm_sound_count++;

	if(alarm_sound_count >= ALARM_BEEP_LENGTH) {
		if(alarm_sound_level == 1) {
			OCR2B = 5;
			alarm_sound_level = 0;
		}
		else {
			OCR2B = ALARM_BEEP_VOLUME;
			alarm_sound_level = 1;
			alarm_current_beep_numer++;
		}

		alarm_sound_count = 0;

		// make sure we don't beep for all eternity if nobody thinks of switching off ...
		if(alarm_current_beep_numer >= ALARM_MAX_BEEPS) {
			alarm_stop_sound();
		}
	}
}
