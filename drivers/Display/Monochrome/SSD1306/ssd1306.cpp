#include "ssd1306.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd;

void SSD13XX::setpos_unsafe(const Vector2i & pos){
    // auto & frame = fetchFrame();
    // frame.setpos_unsafe(pos);
    DEBUG_PRINTLN("not implemented");
}

void SSD13XX::init(){   
    interface.init(); 
    preinitByCmds();
    enable();
    setOffset();
}

void SSD13XX::update(){
    auto & frame = fetchFrame();
    for(int i = 0; i < size().y;i += 8){
        setFlushPos(Vector2i(0, i));
        interface.writeU8(&frame[(i / 8) * size_t(size().x)], size().x);
    }
}

void SSD13XX_72X40::preinitByCmds(){
    scexpr uint8_t initCmds[] = { 
        0xAE,0xD5,0xF0,0xA8,0X27,0XD3,0X00,0X40,
        0X8D,0X14,0X20,0X02,0XA1,0XC8,0XDA,0X12,
        0XAD,0X30,0X81,0XFF,0XD9,0X22,0XDB,0X20,
        0XA4,0XA6,0X0C,0X11
    };

    for(auto & cmd:initCmds){
        interface.writeCommand(cmd);
    }
}

void SSD13XX_128X64::preinitByCmds(){
    // scexpr uint8_t initCmds[] = { 
    //     0x0ae,        /* display off, sleep mode */
    //     0x0d5, 0x010,     /* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
    //     0x0a8, 0x03f,     /* multiplex ratio */
    //     0x0d3, 0x000,     /* display offset */
    //     0x040,        /* start line */
    //     0x08d, 0x010,     /* charge pump setting (p62): 0x014 enable, 0x010 disable */
    //     0x0a1,        /* segment remap a0/a1*/
    //     0x0c8,        /* c0: scan dir normal, c8: reverse */
    //     0x0da, 0x012,     /* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
    //     0x081, 0x0ff,     /* set contrast control */
    //     0x0d9, 0x001,     /* pre-charge period */
    //     0x0db, 0x040,     /* vcomh deselect level */
    //     0x022, 0x000,     /* page addressing mode WRONG: 3 byte cmd! */
    //     0x0a4,        /* output ram to display */
    //     0x0a6,        /* none inverted normal display mode */
    //     0x0af,        /* display on */
    // };
    scexpr uint8_t initCmds[] = { 
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xFF, 0xD9, 0x01, 0xDB, 0x30, 0xA4, 0xA6, 0x8D, 0x14, 0xAF
    };

    for(auto & cmd:initCmds){
        interface.writeCommand(cmd);
    }
}

void SSD13XX_128X32::preinitByCmds(){
    scexpr uint8_t initCmds[] = { 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    };

    for(auto & cmd:initCmds){
        interface.writeCommand(cmd);
    }
}

void SSD13XX_88X48::preinitByCmds(){
    scexpr uint8_t initCmds[] = { 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    };

    for(auto & cmd:initCmds){
        interface.writeCommand(cmd);
    }
}

void SSD13XX_64X48::preinitByCmds(){
    scexpr uint8_t initCmds[] = { 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    };

    for(auto & cmd:initCmds){
        interface.writeCommand(cmd);
    }
}

void SSD13XX_128X80::preinitByCmds(){
    scexpr uint8_t initCmds[] = { 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    };

    for(auto & cmd:initCmds){
        interface.writeCommand(cmd);
    }
}