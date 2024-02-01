#include "st7789.hpp"

void ST7789V2::setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /* ָ��X����������� */
    x1 += x_offset;
    x2 += x_offset;

    writeCommand(0x2a);
    writeData(x1 >> 8);
    writeData(x1);
    writeData(x2 >> 8);
    writeData(x2);

    /* ָ��Y����������� */
    y1 += y_offset;
    y2 += y_offset;

    writeCommand(0x2b);
    writeData(y1 >> 8);
    writeData(y1);
    writeData(y2 >> 8);
    writeData(y2);

    /* ���͸����LCD��ʼ�ȴ������Դ����� */
    writeCommand(0x2C);
}

void ST7789V2::init(){
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

void ST7789V2::flush(RGB565 color){
    setAddress(0, 0, w - 1, h - 1);
    writePool((uint16_t)color, w * h);
}