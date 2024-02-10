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

void ST7789::setArea_Unsafe(const uint16_t & x, const uint16_t & y, const uint16_t & w, const uint16_t & h){
    uint16_t x_offset = offset.x;
    uint16_t y_offset = offset.y;
    uint16_t x1 = x + x_offset;
    uint16_t x2 = x + w + x_offset - 1;
    uint16_t y1 = y + y_offset;
    uint16_t y2 = y + h + y_offset - 1;

    writeCommand(0x2a);
    writeData16(x1);
    writeData16(x2);

    writeCommand(0x2b);
    writeData16(y1);
    writeData16(y2);
}

void ST7789::setPosition_Unsafe(const uint16_t & x, const uint16_t & y){
    writeCommand(0x2a);
    writeData16(x + offset.x);

    writeCommand(0x2b);
    writeData16(y + offset.y);
}