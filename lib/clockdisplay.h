#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <avr/io.h>

void display_init();

void display_time();
void display_alarm();

void paint_line(uint8_t line);
void put_number(uint8_t numIndex, uint8_t slot);

void put_time(uint8_t hours, uint8_t minutes);
void put_time_bcd(uint8_t hours, uint8_t minutes);

void put_status(uint16_t status);
void set_statusbit(uint8_t bit);
void clear_statusbit(uint8_t bit);

#define ANIMATION_DIR_NONE	0
#define ANIMATION_DIR_IN 		1
#define ANIMATION_DIR_OUT		2

void start_fade_in();
void start_fade_out();
void keep_display_on();
void animate();

#endif
