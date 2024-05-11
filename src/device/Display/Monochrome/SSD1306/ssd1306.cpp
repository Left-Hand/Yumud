#include "ssd1306.hpp"


void SSD13XX::init(){    
    preinitByCmds();
    flush(false);
    enable();
}


void SSD13XX::flush(const Binary & color){
    auto & frame = fetchFrame();
    frame.putRect_Unsafe(Rect2i(Vector2i(), this->size), color);
    update();
}

void SSD13XX::update(){
    auto & frame = fetchFrame();
    for(int i = 0; i < size.y;i += 8){
        setFlushPos(Vector2i(0, i));
        interface.writePool(&frame[(i / 8) * size.x], size.x);
    }
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