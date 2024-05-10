#ifndef __ST7789_HPP__

#define __ST7789_HPP__

#include "device_defs.h"
#include "types/image/image.hpp"


class ST7789:public ImageWritable<RGB565>{
private:
    SpiDrv & bus_drv;
    GpioConcept & dc_pin;
    GpioConcept & rst_pin;

    Vector2i offset;

    uint32_t last_point_index = 0;

    bool area_locked = false;
    uint8_t scr_ctrl = 0;
    __fast_inline void writeCommand(const uint8_t & cmd){
        // ST7789_ON_CMD;
        dc_pin.clr();
        bus_drv.write(cmd);
    }

    __fast_inline void writeData(const uint8_t & data){
        // ST7789_ON_DATA;
        dc_pin.set();
        bus_drv.write(data);
    }

    __fast_inline void writeData16(const uint16_t & data){
        // ST7789_ON_DATA;
        dc_pin.set();
        bus_drv.write(data);
    }
    __fast_inline void writePixel(const RGB565 & data){
        // ST7789_ON_DATA;
        dc_pin.set();
        bus_drv.write((uint16_t)data);
    }
    void putPixels(const RGB565 & data, const size_t & len){
        // ST7789_ON_DATA;
        dc_pin.set();
        bus_drv.write((uint16_t)data, len);
    }

    void putPixels(const RGB565 * data_ptr, const size_t & len){
        // ST7789_ON_DATA;
        dc_pin.set();
        bus_drv.write((uint16_t *)data_ptr, len);
    }

    void modifyCtrl(const bool & yes,const uint8_t & pos){
        uint8_t temp = 0x01 << pos;
        if (yes) scr_ctrl |= temp;
        else scr_ctrl &= ~temp;

        writeCommand(0x36);
        writeData(scr_ctrl);
    }


    void putTexture_Unsafe(const Rect2i & rect, const RGB565 * color_ptr) override{
        setArea_Unsafe(rect);
        putPixels(color_ptr, rect.get_area());
    }

    void putRect_Unsafe(const Rect2i & rect, const RGB565 & color) override{
        setArea_Unsafe(rect);
        putPixels(color, rect.get_area());
    }

protected:

    __fast_inline uint32_t getPointIndex(const uint16_t & x, const uint16_t & y){
        return (x + y * size.x);
    }
    void setPosition_Unsafe(const Vector2i & pos) override;
    void setArea_Unsafe(const Rect2i & rect) override;

    __fast_inline void putPixel_Unsafe(const Vector2i & pos, const RGB565 & color){
        setPosition_Unsafe(pos);
        writePixel(color);
    }

    // RGB565 takePixel(const Vector2i & pos) const override{return RGB565();}
public:
    ST7789(SpiDrv & _bus_drv, const Vector2i & _size, GpioConcept & _dc_pin,  GpioConcept &  _rst_pin):
            ImageBasics(_size), ImageWritable<RGB565>(_size),bus_drv(_bus_drv), dc_pin(_dc_pin), rst_pin(_rst_pin){;}
    void init();
    RGB565 * getData(){return nullptr;}
    void setDisplayOffset(const Vector2i & _offset){
        offset = _offset;
    }
    // void setDisplayArea(const Rect2i & _area){
    //     area = _area;
    // }
    void putTexture_Unsafe(const Rect2i & rect, const Grayscale * color_ptr){
        setArea_Unsafe(rect);
        for(size_t i = 0; i < (size_t)rect.get_area(); i++){
            writePixel(RGB565(color_ptr[i]));
        }
    }
    void setFlipY(const bool & flip){modifyCtrl(flip, 7);}
    void setFlipX(const bool & flip){modifyCtrl(flip, 6);}
    void setSwapXY(const bool & flip){modifyCtrl(flip, 5);}
    void setFlushDirV(const bool dir){modifyCtrl(dir, 4);}
    void setFormatRGB(const bool is_rgb){modifyCtrl(!is_rgb, 3);}
    void setFlushDirH(const bool dir){modifyCtrl(dir, 2);}

    void setInversion(const bool & inv){writeCommand(0x20 + inv);}

    // void putTexture(const Rect2i & _area, RGB565 * data){
    //     Rect2i area = _area.
    //     if(area.postion.x + area.size.x > width || )
    //     uint16_t x_begin_in_canvas =
    // }
};

#endif