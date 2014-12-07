#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lib/clockcontrol.h"
#include "lib/clockdisplay.h"
#include "lib/alarm.h"
#include "lib/status.h"
#include "lib/persistence.h"

// anzahl der minuten ohne dcf-interrupt, nach der das DCF-statusflag gelöscht wird
#define DCF_LOST_TIMEOUT 60
// anzahl der sekunden ohne periodic-interrupt, nach der das Sync-statusflag gelöscht wird
#define PERIODIC_LOST_TIMEOUT 60
// anzahl der tlc_periodic-ticks ohne uhrzeitänderung, nach der die uhr als stehengeblieben gilt
#define PROGRESS_LOST_TIMEOUT 2

uint8_t alarm_mode = 0;
uint8_t minutes_without_dcf = 0;
uint8_t seconds_without_periodic = 0;
uint8_t ticks_without_progress = 0;

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
	uint8_t last_second = 60;

	while(1) {

		// wenn das ELV-modul ausnahmsweise mal funktioniert und interrupts erzeugt, werden die
		// entsprechenden status-leds angemacht.
		if(dcf_is_dcfupdate()) {
			dcf_clear_dcfupdate();
			minutes_without_dcf = 0;
			set_statusbit(STATUS_DCF);
		}

		if(dcf_is_periodic()) {
			dcf_clear_periodic();
			seconds_without_periodic = 0;
			set_statusbit(STATUS_PERIODIC);
		}

		/*
		 * Den Mist mit dem update anhand des tlc-ticks braucht es deshalb, weil die periodischen interrupts des dcf-moduls
		 * nicht zuverlässig sind. Deswegen wird anhand des Display-Refresh etwa alle Sekunde die Zeit ausgelesen und gecheckt,
		 * ob wir schon in einer neuen Minute sind.
		 */
		if(tlc_is_periodic()) {
			tlc_clear_periodic();
			update_time();

			if(seconds_without_periodic >= PERIODIC_LOST_TIMEOUT) {
				clear_statusbit(STATUS_PERIODIC);
			}
			else {
				seconds_without_periodic++;
			}

			uint8_t second = time_get_seconds();
			if(second != last_second) {
				last_second = second;
				ticks_without_progress = PROGRESS_LOST_TIMEOUT;
				set_statusbit(STATUS_PROGRESS);
			}
			else if(ticks_without_progress > 0){
				ticks_without_progress--;
			}
			else {
				clear_statusbit(STATUS_PROGRESS);
			}


			uint8_t minute = time_get_minutes();

			// wir sind in einer neuen minute angekommen. alarmbedingungen checken!
			if(minute != last_minute) {
				last_minute = minute;

				if(alarm_is_enabled()) {
					if((alarm_get_hours() == time_get_hours()) && (alarm_get_minutes() == minute)) {
						alarm_start_sound();
					}
				}

				// falls das verkackte DCF-modul länger als DCF_LOST_TIMEOUT kein update mehr gemacht hat,
				// wird die DCF-sync-LED ausgemacht.
				if(minutes_without_dcf >= DCF_LOST_TIMEOUT) {
					clear_statusbit(STATUS_DCF);
				}
				else {
					minutes_without_dcf++;
				}
			}
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

