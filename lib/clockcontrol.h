#ifndef CLOCKCONTROL_H
#define CLOCKCONTROL_H

#include <avr/io.h>

#define DCF_ADDRESS					0x01

#define DCF_REG_SECOND			0x00
#define DCF_REG_MINUTE			0x01
#define DCF_REG_HOUR				0x02
#define DCF_REG_DAYOFWEEK		0x03
#define DCF_REG_DAYOFMONTH	0x04
#define DCF_REG_MONTH				0x05
#define DCF_REG_YEAR				0x06
#define DCF_REG_ALRM_MINUTE	0x07
#define DCF_REG_ALRM_HOUR		0x08
#define DCF_REG_ALRM_DAYS		0x09
#define DCF_REG_INT_MODE		0x0A
#define DCF_REG_ALRM_CONFIG	0x0B
#define DCF_REG_INT_CONFIG	0x0C
#define DCF_REG_DCF_CONFIG	0x0D
#define DCF_REG_STATUS			0x0E
#define DCF_REGBIT_STATUS_DCFIF	0x00
#define DCF_REGBIT_STATUS_PIF		0x01
#define DCF_REGBIT_STATUS_AIF		0x02

#define DCF_DDR							DDRC
#define DCF_INPORT					PINC
#define DCF_PIN_DCF					PC3
#define DCF_PIN_PER					PC2
#define DCF_PIN_ALARM				PC1

/*
 no defines for interrups. attach alarm to int0/pd2 and periodic to int1/pd3.
 */

uint8_t time_get_hours_bcd();
uint8_t time_get_minutes_bcd();
uint8_t time_get_hours();
uint8_t time_get_minutes();
void time_set_time_bcd(uint8_t hours, uint8_t minutes);

void clock_init();
uint8_t read_dcf_reg(uint8_t reg);
void write_dcf_reg(uint8_t reg, uint8_t val);
void update_time();

void dcf_reset_dcfreceive();
void dcf_enable_dcfupdate(uint8_t state);
uint8_t dcf_is_dcfupdate();
void dcf_clear_dcfupdate();

void dcf_enable_alarm(uint8_t state);
uint8_t dcf_is_alarm();
void dcf_clear_alarm();

void dcf_enable_periodic(uint8_t state);
uint8_t dcf_is_periodic();
void dcf_clear_periodic();

#endif


