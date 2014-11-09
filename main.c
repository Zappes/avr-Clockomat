#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lib/clockcontrol.h"
#include "lib/clockdisplay.h"
#include "lib/alarm.h"
#include "lib/status.h"
#include "lib/persistence.h"

#define DCF_LOST_TIMEOUT 60

// Wenn der DCF-Empfang nicht klappt, wird alle
// n Minuten der DCF-Empfänger resettet. Diese
// Zahl hier gibt das n an.
#define DCF_RESET_INTERVAL 15

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

	uint8_t last_minute = 60;

	while(1) {
		if(dcf_is_periodic()) {
			dcf_clear_periodic();
			update_time();

			/*
			 * Den Mist mit "last_minute" braucht es deshalb, weil die periodischen interrupts des dcf-moduls
			 * nicht zuverlässig sind. Deshalb ist es so gemacht, dass der sekündliche Interrupt abgegriffen wird,
			 * der Alarm wird aber nur ausgelöst, wenn sich die Minute im Vergleich zum letzten Interrupt geändert hat.
			 */
			uint8_t minute = time_get_minutes();

			if(minute != last_minute) {
				last_minute = minute;

				if(alarm_is_enabled()) {
					if((alarm_get_hours() == time_get_hours()) && (alarm_get_minutes() == minute)) {
						alarm_start_sound();
					}
				}

				if(minutes_without_dcf >= DCF_LOST_TIMEOUT) {
					clear_statusbit(STATUS_DCF);

					if(minutes_without_dcf % DCF_RESET_INTERVAL == 0) {
						dcf_reset_dcfreceive();
					}
				}
				else {
					minutes_without_dcf++;
				}
			}
		}

		if(dcf_is_dcfupdate()) {
			dcf_clear_dcfupdate();
			minutes_without_dcf = 0;
			set_statusbit(STATUS_DCF);
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

