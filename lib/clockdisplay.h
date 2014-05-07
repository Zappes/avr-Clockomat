#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <avr/io.h>

void putNumber(uint8_t numIndex, uint8_t slot);
void clock_init();
void paintLine(uint8_t line);
void putTime(uint8_t hours, uint8_t minutes);

#endif
