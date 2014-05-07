/*
 
 tlc5940.c
 
 Copyright 2010 Matthew T. Pandina. All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY MATTHEW T. PANDINA "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 EVENT SHALL MATTHEW T. PANDINA OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include <avr/interrupt.h>

#include "tlc5940.h"
#include "clockdisplay.h"

#if (3 * 16 * TLC5940_N > 255)
#define channel3_t uint16_t
#else
#define channel3_t uint8_t
#endif

uint8_t gsData[gsDataSize];

void TLC5940_SetGS(channel_t channel, uint16_t value) {
	channel = numChannels - 1 - channel;
	channel3_t i = (channel3_t) channel * 3 / 2;
	
	switch (channel % 2) {
		case 0:
			gsData[i] = (value >> 4);
			i++;
			gsData[i] = (gsData[i] & 0x0F) | (uint8_t) (value << 4);
			break;
		default: // case 1:
			gsData[i] = (gsData[i] & 0xF0) | (value >> 8);
			i++;
			gsData[i] = (uint8_t) value;
			break;
	}
}

void TLC5940_SetAllGS(uint16_t value) {
	uint8_t tmp1 = (value >> 4);
	uint8_t tmp2 = (uint8_t) (value << 4) | (tmp1 >> 4);
	gsData_t i = 0;
	do {
		gsData[i++] = tmp1;              // bits: 11 10 09 08 07 06 05 04
		gsData[i++] = tmp2;              // bits: 03 02 01 00 11 10 09 08
		gsData[i++] = (uint8_t) value;    // bits: 07 06 05 04 03 02 01 00
	} while (i < gsDataSize);
}

void TLC5940_Init(void) {
	setOutput(SCLK_DDR, SCLK_PIN);
	setOutput(DCPRG_DDR, DCPRG_PIN);
	setOutput(VPRG_DDR, VPRG_PIN);
	setOutput(XLAT_DDR, XLAT_PIN);
	setOutput(BLANK_DDR, BLANK_PIN);
	setOutput(SIN_DDR, SIN_PIN);
	
	setLow(SCLK_PORT, SCLK_PIN);
	setLow(DCPRG_PORT, DCPRG_PIN);
	setHigh(VPRG_PORT, VPRG_PIN);
	setLow(XLAT_PORT, XLAT_PIN);
	setHigh(BLANK_PORT, BLANK_PIN);
	
	// Enable SPI, Master, set clock rate fck/2
	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR = (1 << SPI2X);
	
	// CTC with OCR0A as TOP
	TCCR0A = (1 << WGM01);
	
	// clk_io/1024 (From prescaler)
	TCCR0B = ((1 << CS02) | (1 << CS00));
	
	// Generate an interrupt every 4096 clock cycles
	OCR0A = 3;
	
	// Enable Timer/Counter0 Compare Match A interrupt
	TIMSK0 |= (1 << OCIE0A);
}

// This interrupt will get called every 4096 clock cycles
ISR(TIMER0_COMPA_vect) {
	static uint8_t xlatNeedsPulse = 0;
	static uint8_t scanRow = 0;
	
	setHigh(BLANK_PORT, BLANK_PIN);
	
	if (outputState(VPRG_PORT, VPRG_PIN)) {
		setLow(VPRG_PORT, VPRG_PIN);
		if (xlatNeedsPulse) {
			pulse(XLAT_PORT, XLAT_PIN);
			xlatNeedsPulse = 0;
		}
		pulse(SCLK_PORT, SCLK_PIN);
	} else if (xlatNeedsPulse) {
		pulse(XLAT_PORT, XLAT_PIN);
		xlatNeedsPulse = 0;
	}
	
	setLow(BLANK_PORT, BLANK_PIN);
	
	SPDR = (1 << scanRow);
	paintLine(scanRow);

	if (scanRow < 5) {
		scanRow++;
	} else {
		scanRow = 0;
	}

	while (!(SPSR & (1 << SPIF)));
	
	for (gsData_t i = 0; i < gsDataSize; i++) {
		SPDR = gsData[i];
		while (!(SPSR & (1 << SPIF)));
	}
	xlatNeedsPulse = 1;
}
