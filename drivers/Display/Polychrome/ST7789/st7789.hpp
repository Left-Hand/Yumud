#pragma once

#include "drivers/Display/DisplayerPhy.hpp"
#include "types/image/image.hpp"
#include "hal/bus/spi/spi.hpp"

namespace ymd::drivers{

class ST7789:public Displayer<RGB565>{
private:
    DisplayerPhySpi interface;

    Vector2i offset;

    uint32_t last_point_index = 0;

    bool area_locked = false;
    uint8_t scr_ctrl = 0;

    __fast_inline void writeCommand(const uint8_t cmd){
        interface.writeCommand(cmd);
    }

    __fast_inline void writeData(const uint8_t data){
        interface.writeData(data);
    }

    __fast_inline void writeData16(const uint16_t data){
        interface.writeData16(data);
    }

    void modifyCtrl(const bool yes,const uint8_t pos){
        uint8_t temp = 0x01 << pos;
        if (yes) scr_ctrl |= temp;
        else scr_ctrl &= ~temp;

        writeCommand(0x36);
        writeData(scr_ctrl);
    }



protected:

    __fast_inline uint32_t getPointIndex(const uint16_t x, const uint16_t y){
        return (x + y * size_t(size().x));
    }

    void setpos_unsafe(const Vector2i & pos) override;
    void setarea_unsafe(const Rect2i & rect) override;

    __fast_inline void putpixel_unsafe(const Vector2i & pos, const RGB565 color){
        setpos_unsafe(pos);
        interface.writeData16(uint16_t(color));
    }

    void putrect_unsafe(const Rect2i & rect, const RGB565 color) override;
    void puttexture_unsafe(const Rect2i & rect, const RGB565 * color_ptr) override;
    void putseg_v8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color) override;
    void putseg_h8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color) override;
public:
    ST7789(const DisplayerPhySpi & _interface, const Vector2i & _size):
            ImageBasics(_size), Displayer<RGB565>(_size), interface(_interface){;}

    ST7789(DisplayerPhySpi && _interface, const Vector2i & _size):
            ImageBasics(_size), Displayer<RGB565>(_size), interface(std::move(_interface)){;}

    void init();

    void putTexture(const Rect2i & rect, const is_color auto * color_ptr){
        setarea_unsafe(rect);
        interface.writeMulti<RGB565>(color_ptr, int(rect));
    }

    void setDisplayOffset(const Vector2i & _offset){offset = _offset;}
    void setFlipY(const bool flip){modifyCtrl(flip, 7);}
    void setFlipX(const bool flip){modifyCtrl(flip, 6);}
    void setSwapXY(const bool flip){modifyCtrl(flip, 5);}
    void setFlushDirV(const bool dir){modifyCtrl(dir, 4);}
    void setFormatRGB(const bool is_rgb){modifyCtrl(!is_rgb, 3);}
    void setFlushDirH(const bool dir){modifyCtrl(dir, 2);}

    void setInversion(const bool inv){writeCommand(0x20 + inv);}
};

};