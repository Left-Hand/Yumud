#ifndef __ST7789V2_HPP__

#define __ST7789V2_HPP__

#include "../bus/spi/spi2.hpp"
#include "../../types/rgb.h"

#define ST7789V2_DC_Port SPI2_Port
#define ST7789V2_DC_Pin SPI2_MISO_Pin

#define ST7789V2_ON_DATA \
ST7789V2_DC_Port -> BSHR = ST7789V2_DC_Pin;

#define ST7789V2_ON_CMD \
ST7789V2_DC_Port -> BCR = ST7789V2_DC_Pin;


class ST7789V2{
private:
    Spi2 & spibus;

    uint16_t w = 32;
    uint16_t h = 32;
    uint16_t x_offset = 0;
    uint16_t y_offset = 0;

    __fast_inline void writeCommand(const uint8_t & cmd){
        ST7789V2_ON_CMD;
        spibus.begin();
        spibus.write(cmd);
        spibus.end();
    }

    __fast_inline void writeData(const uint8_t & data){
        ST7789V2_ON_DATA;
        spibus.begin();
        spibus.write(data);
        spibus.end();
    }

    void writePool(uint8_t * data, const size_t & len){
        ST7789V2_ON_DATA;
        spibus.begin();
        spibus.write(data, len);
        spibus.end();
    }

    void writePool(const uint8_t & data, const size_t & len){
        ST7789V2_ON_DATA;
        spibus.begin();
        for(size_t _ = 0; _ < len; _++) spibus.write(data);
        spibus.end();
    }

    void writePool(const uint16_t & data, const size_t & len){
        ST7789V2_ON_DATA;
        spibus.begin();
        spibus.configDataSize(16);
        for(size_t _ = 0; _ < len; _++) spibus.write((uint16_t)(data)); 
        spibus.configDataSize(8);
        spibus.end();
    }

    void setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

public:
    ST7789V2(Spi2 & _spibus):spibus(_spibus){;}
    void init();
    void flush(RGB565 color);     

    void setDisplayArea(const uint16_t & _w, const uint16_t & _h, uint16_t _x_offset = 0, const uint16_t _y_offset = 0){
        w = _w;
        h = _h;
        x_offset = _x_offset;
        y_offset = _y_offset;
    }

    enum class Rotation{
        Rot0 = 0x00,
        Rot90 = 0xC0,
        Rot180 = 0xA0,
        Rot270 = 0x60,
        Rot360 = 0x70
    };

    void setRotation(const Rotation & rot){
        writeCommand(0x36);
        writeData((uint8_t)rot);
    }

    enum class Inversion{
        Disable = 0x20,
        Enable = 0x21
    };

    void setInversion(const Inversion & inv){
        writeCommand((uint8_t)inv);
    }   
};
#endif