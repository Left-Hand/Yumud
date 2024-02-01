#include "st7735.hpp"

void ST7735::setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /* 指定X方向操作区域 */
    writeCommand(0x2a);
    writeData(x1 >> 8);
    writeData(x1);
    writeData(x2 >> 8);
    writeData(x2);

    /* 指定Y方向操作区域 */
    writeCommand(0x2b);
    writeData(y1 >> 8);
    writeData(y1);
    writeData(y2 >> 8);
    writeData(y2);

    /* 发送该命令，LCD开始等待接收显存数据 */
    writeCommand(0x2C);
}

void ST7735::init(){
    spi2.init(144000000/8);

    // delay(120);


	writeCommand(0x01); // SWRESET Software reset
	// delay_ms(20);
    delay(20);
 
    writeCommand(0x11); //Sleep out
    // delay_ms(50);   
    delay(50);

  //----ST7735S Frame Rate---------------------//
    writeCommand(0xB1); //Frame rate 80Hz Frame rate=333k/((RTNA + 20) x (LINE + FPA + BPA))
    writeData(0x02);  //RTNA
    writeData(0x35);  //FPA
    writeData(0x36);   //BPA
    writeCommand(0xB2); //Frame rate 80Hz
    writeData(0x02);
    writeData(0x35);
    writeData(0x36);
    writeCommand(0xB3); //Frame rate 80Hz
    writeData(0x02);
    writeData(0x35);
    writeData(0x36);
    writeData(0x02);
    writeData(0x35);
    writeData(0x36);
 
    //------------------------------------Display Inversion Control-----------------------------------------//
    writeCommand(0xB4);  
    writeData(0x03);
 
    //------------------------------------ST7735S Power Sequence-----------------------------------------//
    writeCommand(0xC0);
    writeData(0xA2);
    writeData(0x02);
    writeData(0x84);
    writeCommand(0xC1);
    writeData(0xC5);
    writeCommand(0xC2);
    writeData(0x0D);
    writeData(0x00);
    writeCommand(0xC3);
    writeData(0x8D);
    writeData(0x2A);
    writeCommand(0xC4);
    writeData(0x8D);
    writeData(0xEE);
    //---------------------------------End ST7735S Power Sequence---------------------------------------//
    writeCommand(0xC5); //VCOM
    writeData(0x0a);
    writeCommand(0x36); //MX, MY, RGB mode
    writeData(0xEC);
    //------------------------------------ST7735S Gamma Sequence-----------------------------------------//
    writeCommand(0XE0);
    writeData(0x12);
    writeData(0x1C);
    writeData(0x10);
    writeData(0x18);
    writeData(0x33);
    writeData(0x2C);
    writeData(0x25);
    writeData(0x28);
    writeData(0x28);
    writeData(0x27);
    writeData(0x2F);
    writeData(0x3C);
    writeData(0x00);
    writeData(0x03);
    writeData(0x03);
    writeData(0x10);
    writeCommand(0XE1);
    writeData(0x12);
    writeData(0x1C);
    writeData(0x10);
    writeData(0x18);
    writeData(0x2D);
    writeData(0x28);
    writeData(0x23);
    writeData(0x28);
    writeData(0x28);
    writeData(0x26);
    writeData(0x2F);
    writeData(0x3B);
    writeData(0x00);
    writeData(0x03);
    writeData(0x03);
    writeData(0x10);
    //------------------------------------End ST7735S Gamma Sequence-----------------------------------------//
    writeCommand(0x3A); //65k mode
    writeData(0x05);
    writeCommand(0x29); //Display on
}

void ST7735::flush(RGB565 color){
    setAddress(0, 0, w - 1, h - 1);
    
    writePool((uint16_t)color, w * h);
}