#ifndef CLOCKALARM_H
#define CLOCKALARM_H

#define ALARM_DDR						DDRD
#define ALARM_INPORT				PIND
#define ALARM_OUTPORT				PORTD
#define ALARM_SHOW_BUTTON		PD7
#define ALARM_ENABLE_BUTTON	PD4
#define ALARM_INC						PD5
#define ALARM_DEC						PD6
#define ALARM_SOUNDPIN			PD3

#define ALARM_BEEP_LENGTH		2000
#define ALARM_BEEP_VOLUME		200
#define ALARM_MAX_BEEPS			200

void alarm_init();
uint8_t alarm_is_shown();
void alarm_update_enable();
uint8_t alarm_get_hours();
uint8_t alarm_get_minutes();
uint8_t alarm_is_enabled();

void alarm_increment();
void alarm_decrement();
void alarm_enable_setting(uint8_t enable);

void alarm_start_sound();
void alarm_stop_sound();
uint8_t alarm_is_sound_started();

#endif
