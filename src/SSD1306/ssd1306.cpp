#include "ssd1306.hpp"


void SSD1306::init(){
    spibus.begin();
    
    static const uint8_t initCmds[] = { 
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x30, 0xA4, 0xA6, 0x8D, 0x14, 0xAF
    };


    for(uint8_t cmd:initCmds){
        writeCommand(cmd);
    }
}


void SSD1306::flush(bool Color){
    volatile static uint8_t t = 0;
    for(uint8_t y = 0; y < h; y+=8)  
    {  
        setPos(0,y);
        // for(uint8_t x = 0; x < w; x++){
            // writeData((uint8_t)((uint16_t)x + (uint16_t)y + (uint16_t)(t++)));
        // }
        writePool((uint8_t)(y + t), (size_t)w);
        t++;
            // __nopn(30);

    }
}