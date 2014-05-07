#include "clockdisplay.h"
#include "tlc5940.h"

uint16_t color_fg = 4095;
uint16_t color_bg = 0;

uint16_t linebuffer[5];
uint16_t numbers[] = {
0b0111101101101111,
0b0001001001001001,
0b0111001111100111,
0b0111001111001111,
0b0101101111001001,
0b0111100111001111,
0b0111100111101111,
0b0111001001001001,
0b0111101111101111,
0b0111101111001001
};

void clock_init() {
	// initialize the LED controller
	TLC5940_Init();

	// Default all channels to off
	TLC5940_SetAllGS(0);
}

void putNumber(uint8_t numIndex, uint8_t slot) {
	uint8_t slotStartBit = (slot << 2);

	for(uint8_t line = 0; line < 5; line++) {
		for(uint8_t numBit = 0; numBit < 3; numBit++) {
			uint8_t numBitPos = (12 + numBit) - (line << 1) - line;

			if(numbers[numIndex] & (1 << numBitPos)) {
				linebuffer[line] |= (1 << (slotStartBit + numBit));
			}	else {
				linebuffer[line] &= ~(1 << (slotStartBit + numBit));
			}
		}
	}
}

void putTime(uint8_t hours, uint8_t minutes) {
	if(hours > 23) hours = 0;
	if(minutes > 59) minutes = 0;

	putNumber(minutes % 10, 0);
	putNumber(minutes / 10, 1);
	putNumber(hours % 10, 2);
	putNumber(hours / 10, 3);
}

void paintLine(uint8_t line) {
	for(uint8_t i = 0; i < 16; i++) {
		TLC5940_SetGS(i, (linebuffer[line] & (1 << i)) ? color_fg : color_bg);
	}
}
