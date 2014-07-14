#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "clockcontrol.h"
#include "i2cmaster.h"

uint8_t time_hours_bcd	=0;
uint8_t time_minutes_bcd=0;

void clock_init() {
	i2c_init();

	// set the three pins for the interrupt lines as inputs.
	DCF_DDR &= ~(_BV(DCF_PIN_ALARM) | _BV(DCF_PIN_DCF) | _BV(DCF_PIN_PER));
}

uint8_t read_dcf_reg(uint8_t reg) {
	i2c_start_wait((DCF_ADDRESS << 1)+I2C_WRITE);

	i2c_write(reg);
	i2c_rep_start((DCF_ADDRESS << 1)+I2C_READ);

	uint8_t ret = i2c_readNak();
	i2c_stop();

	return ret;
}

void write_dcf_reg(uint8_t reg, uint8_t val) {
	i2c_start_wait((DCF_ADDRESS << 1)+I2C_WRITE);

	i2c_write(reg);
	i2c_write(val);
	i2c_stop();
}

uint8_t time_get_hours_bcd() {
	return time_hours_bcd;
}

uint8_t time_get_minutes_bcd() {
	return time_minutes_bcd;
}

uint8_t time_get_hours() {
	return (((time_hours_bcd & 0xf0) >> 4) * 10) + (time_hours_bcd & 0x0f);
}

uint8_t time_get_minutes() {
	return (((time_minutes_bcd & 0xf0) >> 4 ) * 10) + (time_minutes_bcd & 0x0f);
}

void time_set_time_bcd(uint8_t hours, uint8_t minutes) {
	time_hours_bcd = hours;
	time_minutes_bcd = minutes;
}

void update_time() {
	i2c_start_wait((DCF_ADDRESS << 1)+I2C_WRITE);

	i2c_write(DCF_REG_MINUTE);
	i2c_rep_start((DCF_ADDRESS << 1)+I2C_READ);

	uint8_t minute = i2c_readAck();
	uint8_t hour = i2c_readNak();
	i2c_stop();

	time_set_time_bcd(hour, minute);
}

void dcf_enable_dcfupdate(uint8_t state) {
	if(state == 1) {
		dcf_clear_dcfupdate();
		write_dcf_reg(DCF_REG_DCF_CONFIG, 3);
	}
	else {
		write_dcf_reg(DCF_REG_DCF_CONFIG, 1);
	}
}
uint8_t dcf_is_dcfupdate() {
	return (DCF_INPORT & _BV(DCF_PIN_DCF)) ? 0 : 1;
}
void dcf_clear_dcfupdate() {
	write_dcf_reg(DCF_REG_STATUS, _BV(DCF_REGBIT_STATUS_DCFIF));
}

void dcf_enable_alarm(uint8_t state) {
	if(state == 1) {
		dcf_clear_alarm();
		write_dcf_reg(DCF_REG_ALRM_CONFIG, 2);
	}
	else {
		write_dcf_reg(DCF_REG_INT_CONFIG, 0);
	}
}
uint8_t dcf_is_alarm() {
	return (DCF_INPORT & _BV(DCF_PIN_ALARM)) ? 0 : 1;
}
void dcf_clear_alarm() {
	write_dcf_reg(DCF_REG_STATUS, _BV(DCF_REGBIT_STATUS_AIF));
}

void dcf_enable_periodic(uint8_t state) {
	if(state == 1) {
		dcf_clear_periodic();
		write_dcf_reg(DCF_REG_INT_MODE, 3);			// periodic interrupt on every minute.
		write_dcf_reg(DCF_REG_INT_CONFIG, 6);		// enable periodic interrupt.
	}
	else {
		write_dcf_reg(DCF_REG_INT_CONFIG, 0);
	}
}
uint8_t dcf_is_periodic() {
	return (DCF_INPORT & _BV(DCF_PIN_PER)) ? 0 : 1;
}
void dcf_clear_periodic() {
	write_dcf_reg(DCF_REG_STATUS, _BV(DCF_REGBIT_STATUS_PIF));
}
