#ifndef __ST7789V2_HPP__

#define __ST7789V2_HPP__

#include "device_defs.h"
#include "types/image/image.hpp"

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
    uint32_t last_point_index = 0;
    bool area_locked = false;

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
    __fast_inline void writePixel(const RGB565 & data){
        ST7789V2_ON_DATA;
        bus_drv.write((uint16_t)data);
    }
    void putPixels(const RGB565 & data, const size_t & len) override{
        ST7789V2_ON_DATA;
        bus_drv.write((uint16_t)data, len);
    }

    void putPixels(const RGB565 * data_ptr, const size_t & len) override{
        ST7789V2_ON_DATA;
        bus_drv.write((uint16_t *)data_ptr, len);
    }

    void modifyCtrl(const bool & yes,const uint8_t & pos){
        uint8_t temp = 0x01 << pos;
        if (yes) scr_ctrl |= temp;
        else scr_ctrl &= ~temp;

        writeCommand(0x36);
        writeData(scr_ctrl);
    }

protected:

    __fast_inline uint32_t getPointIndex(const uint16_t & x, const uint16_t & y){
        return (x + y * area.size.x);
    }
    void setPosition_Unsafe(const Vector2i & pos) override;
    void setArea_Unsafe(const Rect2i & rect) override;
    __fast_inline void putPixel_Unsafe(const Vector2i & pos, const RGB565 & color){
        setPosition_Unsafe(pos);
        writePixel(color);
    }
    __fast_inline void putPixel(const Vector2i & pos, const RGB565 & color) override{
        if(!area.has_point(pos)) return;
        putPixel_Unsafe(pos, color);
    }

    RGB565 takePixel(const Vector2i & pos) const override{return RGB565();}
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