#include "ssd1306.hpp"


void SSD13XX::init(){    
    preinitByCmds();
    clear();
    enable();
    for(uint16_t i = 0; i < 360; i++){
        data[i] = 0x55;
    }
}


void SSD13XX::flush(const Binary & color){
    static uint8_t cnt = 0;
    cnt++;
    for(uint8_t i=0;i<size.y / 8;i++){
        setPos(0,i);

        // interface.writePool(color ? 0xFF : 0x00, (size_t)size.x);
        for(uint8_t j = 0; j < size.x; j++){
            data[i * size.x + j] = i + j + cnt;
            interface.writeData(data[i * size.x + j]);
        }
    }
    // static uint8_t t = 0;
    // t++;

    // for(uint8_t i=0;i<5;i++){
    //     setPos(0,i);
    //     for(uint8_t m = 0; m < 72;m++){
    //         interface.writeData(t);
    //     }
    // }
}

void SSD13XX_72X40::preinitByCmds(){
    static constexpr uint8_t initCmds[] = { 
        0xAE,0xD5,0xF0,0xA8,0X27,0XD3,0X00,0X40,
        0X8D,0X14,0X20,0X02,0XA1,0XC8,0XDA,0X12,
        0XAD,0X30,0X81,0XFF,0XD9,0X22,0XDB,0X20,
        0XA4,0XA6,0X0C,0X11
    };

    for(auto & cmd:initCmds){
        interface.writeCommand(cmd);
    }
}