#include "ssd1306.hpp"


void SSD1306::init(){    
    // static const uint8_t initCmds[] = { 
    //     0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x30, 0xA4, 0xA6, 0x8D, 0x14, 0xAF
    // };
    // static const uint8_t initCmds[] = { 
    //     0xAE, 0xD5, 0xf0, 0xa8, 0x27, 0xd3, 0x00, 0x40, 0x8d, 0x14, 0x20, 0x02, 0xa1, 0xc8, 0xda, 0x12, 0xad, 0x30, 0x81, 0xff, 0xd9, 0x22, 0xdb, 0x20, 0xa4, 0xa6, 0x0c, 0x11, 0xaf
    // };
    static constexpr uint8_t initCmds[] = { 
        0xAE,0xD5,0xF0,0xA8,0X27,0XD3,0X00,0X40,
        0X8D,0X14,0X20,0X02,0XA1,0XC8,0XDA,0X12,
        0XAD,0X30,0X81,0XFF,0XD9,0X22,0XDB,0X20,
        0XA4,0XA6,0X0C,0X11
    };

    for(auto & cmd:initCmds){
        interface.writeCommand(cmd);
    }

    enable();
}


void SSD1306::flush(const Binary & color){
    static uint8_t t = 0;
    t++;

    for(uint8_t i=0;i<5;i++){
        setPos(0,i);
        for(uint8_t m = 0; m < 72;m++){
            interface.writeData(t);
        }
    }

}