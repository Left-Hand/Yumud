#ifndef __ST7789V2_HPP__

#define __ST7789V2_HPP__

#include "../bus/bus_inc.h"
#include "../bus/spi/spi2.hpp"
#include "../types/image/image.hpp"

#define ST7789V2_DC_Port SPI2_Port
#define ST7789V2_DC_Pin SPI2_MISO_Pin

#define ST7789V2_ON_DATA \
ST7789V2_DC_Port -> BSHR = ST7789V2_DC_Pin;

#define ST7789V2_ON_CMD \
ST7789V2_DC_Port -> BCR = ST7789V2_DC_Pin;


class ST7789:public Image565{
private:
    SpiDrv & bus_drv;
    Vector2i offset;
    uint8_t scr_ctrl = 0;

    __fast_inline void writeCommand(const uint8_t & cmd){
        ST7789V2_ON_CMD;
        bus_drv.write(cmd);
    }

    __fast_inline void writeData(const uint8_t & data){
        ST7789V2_ON_DATA;
        bus_drv.write(data);
    }

    __fast_inline void writeData16(const uint16_t & data){
        ST7789V2_ON_DATA;
        bus_drv.write(data);
    }

    __fast_inline void writeDataPool(const uint16_t & data, const size_t len){
        ST7789V2_ON_DATA;
        bus_drv.write(data, len);
    }

    __fast_inline void writeDataPool(const uint16_t * data_ptr, const size_t len){
        ST7789V2_ON_DATA;
        bus_drv.write(data_ptr, len);
    }
    __fast_inline void writeRGB(const RGB565 & data){
        writeCommand(0x2c);
        writeData16(static_cast<uint16_t>(data));
    }
    void writeRGB(const RGB565 & data, const size_t & len){
        writeCommand(0x2c);
        writeDataPool(static_cast<uint16_t>(data), len);
    }

    void writeRGB(const RGB565 * data_ptr, const size_t & len){
        ST7789V2_ON_DATA;
        writeCommand(0x2c);
        bus_drv.write((uint16_t *)(data_ptr), len);
    }

    void modifyCtrl(const bool & yes,const uint8_t & pos){
        uint8_t temp = 0x01 << pos;
        if (yes) scr_ctrl |= temp;
        else scr_ctrl &= ~temp;

        writeCommand(0x36);
        writeData(scr_ctrl);
    }

protected:
    __fast_inline bool pointValid(const uint16_t & x, const uint16_t & y){
        return area.has_point(Vector2i(x,y));
    }
    void setPosition_Unsafe(const uint16_t & x, const uint16_t & y);
    __fast_inline void putPixel_Unsafe(const uint16_t & x, const uint16_t & y, const RGB565 & color){
        setPosition_Unsafe(x,y);
        writeRGB(color);
    }

    void setArea_Unsafe(const uint16_t & x, const uint16_t & y, const uint16_t & w, const uint16_t & h);

    void putTexture_Unsafe(const uint16_t & x, const uint16_t & y, const uint16_t & w, const uint16_t & h, const RGB565 * color_ptr){
        setArea_Unsafe(x, y, w, h);
        writeRGB(color_ptr, w*h);
    }

    void putRect_Unsafe(const uint16_t & x, const uint16_t & y, const uint16_t & w, const uint16_t & h, const RGB565 & color){
        setArea_Unsafe(x, y, w, h);
        writeRGB(color, w*h);
    }

    __fast_inline void putPixel_Unsafe(const Vector2i & pos, const RGB565 & color) override{
        putPixel_Unsafe(pos.x, pos.y, color);
    }

    __fast_inline void putPixel(const Vector2i & pos, const RGB565 & color) override{
        if(!pointValid((uint16_t)pos.x, (uint16_t)pos.y)) return;
        putPixel_Unsafe(pos.x, pos.y, color);
    }

    void putTexture_Unsafe(const Rect2i & _area, const RGB565 * color_ptr) override{
        putTexture_Unsafe(_area.position.x, _area.position.y, _area.size.x, _area.size.y, color_ptr);
    }

    void putRect_Unsafe(const Rect2i & _area, const RGB565 & color) override{
        putRect_Unsafe(_area.position.x, _area.position.y, _area.size.x, _area.size.y, color);
    }

public:
    ST7789(SpiDrv & _bus_drv):bus_drv(_bus_drv){;}
    void init();

    void setDisplayOffset(const Vector2i & _offset){
        offset = _offset;
    }
    void setDisplayArea(const Rect2i & _area){
        area = _area;
    }

    void setFlipY(const bool & flip){modifyCtrl(flip, 7);}
    void setFlipX(const bool & flip){modifyCtrl(flip, 6);}
    void setSwapXY(const bool & flip){modifyCtrl(flip, 5);}
    void setFlushDirV(const bool dir){modifyCtrl(dir, 4);}
    void setFormatRGB(const bool is_rgb){modifyCtrl(!is_rgb, 3);}
    void setFlushDirH(const bool dir){modifyCtrl(dir, 2);}

    void setInversion(const bool & inv){writeCommand(0x20 + inv);}


    void flush(const RGB565 & color){
        putRect_Unsafe(area, color);
    }
    // void putTexture(const Rect2i & _area, RGB565 * data){
    //     Rect2i area = _area.
    //     if(area.postion.x + area.size.x > width || )
    //     uint16_t x_begin_in_canvas =
    // }
};

#endif