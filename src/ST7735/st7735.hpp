#ifndef __ST7735_HPP__

#define __ST7735_HPP__

#include "../bus/spi/spi2.hpp"
#include "../../types/rgb.h"

#define ST7735_DC_Port SPI2_Port
#define ST7735_DC_Pin SPI2_MISO_Pin

#define ST7735_ON_DATA \
ST7735_DC_Port -> BSHR = ST7735_DC_Pin;

#define ST7735_ON_CMD \
ST7735_DC_Port -> BCR = ST7735_DC_Pin;


class ST7735{
private:
    const uint16_t w = 160;
    const uint16_t h = 80;

    __fast_inline void writeCommand(const uint8_t & cmd){
        ST7735_ON_CMD;
        spi2.begin();
        spi2.write(cmd);
        spi2.end();
    }

    __fast_inline void writeData(const uint8_t & data){
        ST7735_ON_DATA;
        spi2.begin();
        spi2.write(data);
        spi2.end();
    }

    void writePool(uint8_t * data, const size_t & len){
        ST7735_ON_DATA;
        spi2.begin();
        spi2.write(data, len);
        spi2.end();
    }

    void writePool(const uint8_t & data, const size_t & len){
        ST7735_ON_DATA;
        spi2.begin();
        for(size_t _ = 0; _ < len; _++) spi2.write(data);
        spi2.end();
    }

    void writePool(const uint16_t & data, const size_t & len){
        ST7735_ON_DATA;
        spi2.begin();
        spi2.configDataSize(16);
        for(size_t _ = 0; _ < len; _++) spi2.write((uint16_t)(data));
        spi2.configDataSize(8);
        spi2.end();
    }

    void setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
public:
    void init();
    void flush(RGB565 color);        
};
#endif