#include "st7789.hpp"


void ST7789::init(){
    writeCommand(0x01);

    delayMicroseconds(50);
	writeCommand(0x11);
    delayMicroseconds(50);
	writeCommand(0x3A);
	writeData(0x55);
	writeCommand(0x36);
	writeData(0x00);
	writeCommand(0x21);
	writeCommand(0x13);
	writeCommand(0x29);
}

