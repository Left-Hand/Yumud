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
  uint8_t i,n;       
  for(i=0xb0;i<0xb5;i++)  
  {  
    writeCommand(i);
    writeCommand(0x00);
    writeCommand(0x10);
    for(n=0;n<128;n++) 
        writeData(Color ? 0xff : 0x00); 
  }
}