/*
 * persistence.h
 *
 *  Created on: 11.07.2014
 *      Author: zaphod
 */

#ifndef PERSISTENCE_H_
#define PERSISTENCE_H_

#include <avr/io.h>

#define EEPROM_MAGIC 0xBEEF

typedef struct {
  uint16_t magic;
  uint8_t alarm_enable;
  uint8_t alarm_hours;
  uint8_t alarm_minutes;
} ClockomatPersistenceData;

void persistence_restore(uint8_t *enable, uint8_t *hours, uint8_t *minutes);
void persistence_persist(uint8_t enable, uint8_t hours, uint8_t minutes);

#endif /* PERSISTENCE_H_ */
