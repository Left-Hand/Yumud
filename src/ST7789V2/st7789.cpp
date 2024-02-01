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

    delayMicroseconds(5);
	writeCommand(0x11);
	
    // delayMicroseconds(5);
	// writeCommand(0x3A);
	// writeData(0x55);
	// writeCommand(0x36);
	// writeData(0x00);
	// writeCommand(0x21);
	// writeCommand(0x13);
	// writeCommand(0x29);
        writeCommand(0xB1); //֡��
        writeData(0x05);
        writeData(0x3A);
        writeData(0x3A);

        writeCommand(0xB2); //֡��
        writeData(0x05);
        writeData(0x3A);
        writeData(0x3A);

        writeCommand(0xB3); //֡��
        writeData(0x05);
        writeData(0x3A);
        writeData(0x3A);
        writeData(0x05);
        writeData(0x3A);
        writeData(0x3A);

        writeCommand(0x21); //������(�õ��ǵ��ԵĻ����Լ�ת����RGB565���Կ�����)
        writeCommand(0xB4); //����control
        writeData(0x03);

        writeCommand(0xC0); //���� control
        writeData(0x62);
        writeData(0x02);
        writeData(0x04);

        writeCommand(0xC1);
        writeData(0xC0);

        writeCommand(0xC2);
        writeData(0x0D);
        writeData(0x00);

        writeCommand(0xC3);
        writeData(0x8D);
        writeData(0x6A);

        writeCommand(0xC4);
        writeData(0x8D);
        writeData(0x6A);

        writeCommand(0xC5); //VCOM
        writeData(0x0E);

        writeCommand(0xE0);
        writeData(0x10);
        writeData(0x0E);
        writeData(0x02);
        writeData(0x03);
        writeData(0x0E);
        writeData(0x07);
        writeData(0x02);
        writeData(0x07);
        writeData(0x0A);
        writeData(0x12);
        writeData(0x27);
        writeData(0x37);
        writeData(0x00);
        writeData(0x0D);
        writeData(0x0E);
        writeData(0x10);

        writeCommand(0xE1);
        writeData(0x10);
        writeData(0x0E);
        writeData(0x03);
        writeData(0x03);
        writeData(0x0F);
        writeData(0x06);
        writeData(0x02);
        writeData(0x08);
        writeData(0x0A);
        writeData(0x13);
        writeData(0x26);
        writeData(0x36);
        writeData(0x00);
        writeData(0x0D);
        writeData(0x0E);
        writeData(0x10);

        writeCommand(0x3A); //��Ӳ�����ѡ��ģʽ
        writeData(0x05);

        writeCommand(0x36); //ˢ�·���
        writeData(0x78);

        writeCommand(0x13); //��λ��Ĭ�Ͽ�����

        // writeCommand(0xB4); //��ʾ����
        // writeData(0x00);
        writeCommand(0x29); //display on
}

void ST7789V2::flush(RGB565 color){
    setAddress(0, 0, w - 1, h - 1);
    
    writePool((uint16_t)color, w * h);
}