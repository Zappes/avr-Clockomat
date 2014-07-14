/*
 * persistence.c
 *
 *  Created on: 11.07.2014
 *      Author: zaphod
 */

#include "persistence.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

ClockomatPersistenceData pers_data = {0,0,0,0};

void persistence_restore(uint8_t *enable, uint8_t *hours, uint8_t *minutes) {

	cli();
	eeprom_read_block((void*)&pers_data, (void*)0, sizeof(pers_data));
	sei();

	if(pers_data.magic == EEPROM_MAGIC) {
		*enable = pers_data.alarm_enable;
		*hours = pers_data.alarm_hours;
		*minutes = pers_data.alarm_minutes;
	}
	else {
		*enable = 0;
		*hours = 7;
		*minutes = 00;
	}
}

void persistence_persist(uint8_t enable, uint8_t hours, uint8_t minutes) {
	// only persist if it differs from the last read state
	if(enable != pers_data.alarm_enable || hours != pers_data.alarm_hours || minutes != pers_data.alarm_minutes) {
		pers_data.magic = EEPROM_MAGIC;
		pers_data.alarm_enable = enable;
		pers_data.alarm_hours = hours;
		pers_data.alarm_minutes = minutes;

		cli();
		eeprom_write_block((void*)&pers_data, (void*)0, sizeof(pers_data));
		sei();
	}
}

