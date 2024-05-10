#ifndef __SSD1306_HPP__
#define __SSD1306_HPP__

#include "../../DisplayerInterface.hpp"

class SSD1306{
protected:
    DisplayerInterface & interface;

    uint16_t width = 72;
    uint16_t height = 40;
    uint16_t x_offset = 0;



    void setPos(uint16_t x,uint16_t y){
        // x += x_offset;
        // y >>= 3;
        // interface.writeCommand(0xb0 + y);
        // interface.writeCommand(((x&0xf0)>>4)|0x10);
        // interface.writeCommand((x&0x0f));
        x+=28;
        interface.writeCommand(0xb0+y);
        interface.writeCommand(((x&0xf0)>>4)|0x10);
        interface.writeCommand((x&0x0f));
    }
public:
    static constexpr uint8_t default_id = 0x78;

    SSD1306(DisplayerInterface & _interface):interface(_interface){;}
    void init();
    void flush(const Binary & color);     

    void setOffsetX(const uint8_t & offset){x_offset = offset;}
    void setOffsetY(const uint8_t & offset){
        interface.writeCommand(0xD3); 
        interface.writeCommand(offset);
    }
    void enable(const bool & en = true){
        if(en){
            interface.writeCommand(0x8D);
            interface.writeCommand(0x14);
            interface.writeCommand(0xAF);
        }else{
            interface.writeCommand(0x8D);
            interface.writeCommand(0x10);
            interface.writeCommand(0xAE);
        }
    }

    void turnDisplay(const bool & i){
        interface.writeCommand(0xC8 - 8*i);//正常显示
        interface.writeCommand(0xA1 - i);
    }
    void enableFlipY(const bool & flip = true){interface.writeCommand(0xA0 | flip);}
    void enableFlipX(const bool & flip = true){interface.writeCommand(0xC0 | (flip << 3));}
    void enableInversion(const bool & inv = true){interface.writeCommand(0xA7 - inv);}  
    void clear(){
  uint8_t i,n;       
  for(i=0xb0;i<0xb5;i++)  
  {  
    interface.writeCommand(i);
    interface.writeCommand(0x0c);
    interface.writeCommand(0x11);    
    for(n=0;n<72;n++) interface.writeData(0); 
  }
}

};


#endif