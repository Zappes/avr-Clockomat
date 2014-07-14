#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lib/clockcontrol.h"
#include "lib/clockdisplay.h"
#include "lib/alarm.h"
#include "lib/status.h"
#include "lib/persistence.h"

#define DCF_LOST_TIMEOUT 15

uint8_t alarm_mode = 0;
uint8_t minutes_without_dcf = 0;

int main(void) {
	put_status(0);

	clock_init();
	display_init();
	alarm_init();

	// Enable Global Interrupts
	sei();

	dcf_enable_alarm(0);
	dcf_enable_dcfupdate(1);
	dcf_enable_periodic(1);

	update_time();

	while(1) {
		if(dcf_is_periodic()) {
			update_time();
			dcf_clear_periodic();

			if(alarm_is_enabled()) {
				if((alarm_get_hours() == time_get_hours()) && (alarm_get_minutes() == time_get_minutes())) {
					alarm_start_sound();
				}
			}

			if(minutes_without_dcf > DCF_LOST_TIMEOUT) {
				clear_statusbit(STATUS_DCF);
			}
			else {
				minutes_without_dcf++;
			}
		}
		if(dcf_is_dcfupdate()) {
			update_time();
			minutes_without_dcf = 0;
			set_statusbit(STATUS_DCF);
			dcf_enable_dcfupdate(0);
			dcf_clear_dcfupdate();
		}

		if(alarm_is_sound_started()) {
			keep_display_on();
		}

		if(alarm_is_shown()) {
			if(alarm_mode == 0) {
				alarm_mode = 1;
				alarm_enable_setting(1);
			}

			display_alarm();
			keep_display_on();

			alarm_update_enable();
			if(alarm_is_enabled()) {
				set_statusbit(STATUS_ALARM);
			}
			else {
				clear_statusbit(STATUS_ALARM);
			}
		}
		else {
			if(alarm_mode == 1) {
				alarm_mode = 0;
				alarm_enable_setting(0);

				persistence_persist(alarm_is_enabled(), alarm_get_hours(), alarm_get_minutes());
			}

			display_time();
		}
	}

	return 0;
}

