#ifndef __SSD1306_HPP__
#define __SSD1306_HPP__


#include "../bus/spi/spi2.hpp"
#include "../../types/rgb.h"

#define SSD1306_DC_Port SPI2_Port
#define SSD1306_DC_Pin SPI2_MISO_Pin

#define SSD1306_ON_DATA \
SSD1306_DC_Port -> BSHR = SSD1306_DC_Pin;

#define SSD1306_ON_CMD \
SSD1306_DC_Port -> BCR = SSD1306_DC_Pin;

class SSD1306{
private:
    Spi & spibus;

    uint16_t w = 128;
    uint16_t h = 80;
    uint16_t x_offset = 0;

    __fast_inline void writeCommand(const uint8_t & cmd){
        SSD1306_ON_CMD;
        spibus.begin();
        spibus.write(cmd);
        spibus.end();
    }

    __fast_inline void writeData(const uint8_t & data){
        SSD1306_ON_DATA;
        spibus.begin();
        spibus.write(data);
        spibus.end();
    }

    void writePool(uint8_t * data, const size_t & len){
        SSD1306_ON_DATA;
        spibus.begin();
        spibus.write(data, len);
        spibus.end();
    }

    void writePool(const uint8_t & data, const size_t & len){
        SSD1306_ON_DATA;
        spibus.begin();
        for(size_t _ = 0; _ < len; _++) spibus.write(data);
        spibus.end();
    }

    void writePool(const uint16_t & data, const size_t & len){
        SSD1306_ON_DATA;
        spibus.begin();
        spibus.configDataSize(16);
        for(size_t _ = 0; _ < len; _++) spibus.write((uint16_t)(data)); 
        spibus.configDataSize(8);
        spibus.end();
    }

    void setPos(uint16_t x,uint16_t y){
        x += x_offset;
        y >>= 3;
        writeCommand(0xb0 + y);
        writeCommand(((x&0xf0)>>4)|0x10);
        writeCommand((x&0x0f));
    }
public:
    SSD1306(Spi2 & _spibus):spibus(_spibus){;}
    void init();
    void flush(bool color);     

    void setOffsetX(const uint8_t offset){x_offset = offset;}
    void setOffsetY(const uint8_t offset){writeCommand(0xD3); writeCommand(offset);}
    void setFlipY(const bool & flip){writeCommand(0xA0 | flip);}
    void setFlipX(const bool & flip){writeCommand(0xC0 | (flip << 3));}
    void setInversion(const bool & inv){writeCommand(0x7A - inv);}  
};


#endif