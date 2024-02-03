#ifndef __ST7789V2_HPP__

#define __ST7789V2_HPP__

#include "../bus/busdrv.hpp"
#include "../bus/spi/spi2.hpp"
#include "../../types/rgb.h"

#define ST7789V2_DC_Port SPI2_Port
#define ST7789V2_DC_Pin SPI2_MISO_Pin

#define ST7789V2_ON_DATA \
ST7789V2_DC_Port -> BSHR = ST7789V2_DC_Pin;

#define ST7789V2_ON_CMD \
ST7789V2_DC_Port -> BCR = ST7789V2_DC_Pin;


class ST7789{
private:
    BusDrv & busdrv;

    uint16_t w = 32;
    uint16_t h = 32;
    uint16_t x_offset = 0;
    uint16_t y_offset = 0;

    uint8_t scr_ctrl = 0;

    __fast_inline void writeCommand(const uint8_t & cmd){
        ST7789V2_ON_CMD;
        busdrv.write(cmd);
    }

    __fast_inline void writeData(const uint8_t & data){
        ST7789V2_ON_DATA;
        busdrv.write(data);
    }

    void writePool(uint8_t * data, const size_t & len){
        ST7789V2_ON_DATA;
        busdrv.write(data, len);
    }

    void writePool(const uint8_t & data, const size_t & len){
        ST7789V2_ON_DATA;
        busdrv.write(data, len);
    }

    void writePool(const uint16_t & data, const size_t & len){
        ST7789V2_ON_DATA;
        busdrv.write(data, len);
    }

    void setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void modifyCtrl(const bool & yes,const uint8_t & pos){
        uint8_t temp = 0x01 << pos;
        if (yes) scr_ctrl |= temp;
        else scr_ctrl &= ~temp;

        writeCommand(0x36);
        writeData(scr_ctrl);
    }
public:
    ST7789(BusDrv & _busdrv):busdrv(_busdrv){;}
    void init();
    void flush(RGB565 color);     

    void setDisplayArea(const uint16_t & _w, const uint16_t & _h, uint16_t _x_offset = 0, const uint16_t _y_offset = 0){
        w = _w;
        h = _h;
        x_offset = _x_offset;
        y_offset = _y_offset;
    }

    void setFlipY(const bool & flip){modifyCtrl(flip, 7);}
    void setFlipX(const bool & flip){modifyCtrl(flip, 6);}
    void setSwapXY(const bool & flip){modifyCtrl(flip, 5);}
    void setReflashDirV(const bool dir){modifyCtrl(dir, 4);}
    void setFormatRGB(const bool isrgb){modifyCtrl(!isrgb, 3);}
    void setReflashDirH(const bool dir){modifyCtrl(dir, 2);}

    void setInversion(const bool & inv){writeCommand(0x20 + inv);}   
};
#endif