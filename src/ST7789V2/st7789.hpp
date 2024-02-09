#ifndef __ST7789V2_HPP__

#define __ST7789V2_HPP__

#include "../bus/bus_inc.h"
#include "../bus/spi/spi2.hpp"
#include "../../types/rgb.h"
#include "../../types/vector2/vector2_t.hpp"
#include "../../types/rect2/rect2_t.hpp"

#define ST7789V2_DC_Port SPI2_Port
#define ST7789V2_DC_Pin SPI2_MISO_Pin

#define ST7789V2_ON_DATA \
ST7789V2_DC_Port -> BSHR = ST7789V2_DC_Pin;

#define ST7789V2_ON_CMD \
ST7789V2_DC_Port -> BCR = ST7789V2_DC_Pin;


class ST7789{
private:
    SpiDrv & bus_drv;

    uint16_t width = 32;
    uint16_t height = 32;
    uint16_t x_offset = 0;
    uint16_t y_offset = 0;

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

    __fast_inline void writeRGB(const RGB565 & data){
        ST7789V2_ON_DATA;
        writeCommand(0x2c);
        bus_drv.write(static_cast<uint16_t>(data));
    }
    void writeRGB(const RGB565 & data, const size_t & len){
        ST7789V2_ON_DATA;
        writeCommand(0x2c);
        bus_drv.write(static_cast<uint16_t>(data), len);
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
        return (x < width && y < height);
    }
    __fast_inline void setPosition_Unsafe(const uint16_t & x, const uint16_t & y){
        writeCommand(0x2a);
        writeData(x + x_offset);

        writeCommand(0x2b);
        writeData(y + y_offset);
    }
    __fast_inline void putPixel_Unsafe(const uint16_t & x, const uint16_t & y, const RGB565 & color){
        setPosition_Unsafe(x,y);
        writeRGB(color);
    }

    void setArea_Unsafe(const uint16_t & x, const uint16_t & y, const uint16_t & w, const uint16_t & h){
        uint16_t x1 = x + x_offset;
        uint16_t x2 = x + w + x_offset;
        uint16_t y1 = y + y_offset;
        uint16_t y2 = y + h + y_offset;

        writeCommand(0x2a);
        writeData16(x1);
        writeData16(x2);

        writeCommand(0x2b);
        writeData16(y1);
        writeData16(y2);
    }

    void putTexture_Unsafe(const uint16_t & x, const uint16_t & y, const uint16_t & w, const uint16_t & h, const RGB565 * color_ptr){
        setArea_Unsafe(x, y, w, h);
        writeRGB(color_ptr, w*h);
    }

    void putTexture_Unsafe(const uint16_t & x, const uint16_t & y, const uint16_t & w, const uint16_t & h, const RGB565 & color){
        setArea_Unsafe(x, y, w, h);
        writeRGB(color, w*h);
    }


public:
    ST7789(SpiDrv & _bus_drv):bus_drv(_bus_drv){;}
    void init();

    void setDisplayArea(const uint16_t & _w, const uint16_t & _h, uint16_t _x_offset = 0, const uint16_t _y_offset = 0){
        width = _w;
        height = _h;
        x_offset = _x_offset;
        y_offset = _y_offset;
    }

    void setFlipY(const bool & flip){modifyCtrl(flip, 7);}
    void setFlipX(const bool & flip){modifyCtrl(flip, 6);}
    void setSwapXY(const bool & flip){modifyCtrl(flip, 5);}
    void setFlushDirV(const bool dir){modifyCtrl(dir, 4);}
    void setFormatRGB(const bool is_rgb){modifyCtrl(!is_rgb, 3);}
    void setFlushDirH(const bool dir){modifyCtrl(dir, 2);}

    void setInversion(const bool & inv){writeCommand(0x20 + inv);}
    __fast_inline void putPixel(const Vector2i & pos, const RGB565 & color){
        if(!pointValid((uint16_t)pos.x, (uint16_t)pos.y)) return;
        putPixel_Unsafe(pos.x, pos.y, color);
    }

    void putTexture_Unsafe(const Rect2i & area, RGB565 * color_ptr){
        setArea_Unsafe(area.position.x, area.position.y, area.size.x, area.size.y);
        writeRGB(color_ptr, area.get_area());
    }

    void putRect_Unsafe(const Rect2i & area, RGB565 & color){
        setArea_Unsafe(area.position.x, area.position.y, area.size.x, area.size.y);
        writeRGB(color, area.get_area());
    }

    void flush(RGB565 color){
        putRect_Unsafe(Rect2i(Vector2i(), Vector2i(width, height)), color);
    }
    // void putTexture(const Rect2i & _area, RGB565 * data){
    //     Rect2i area = _area.
    //     if(area.postion.x + area.size.x > width || )
    //     uint16_t x_begin_in_canvas =
    // }
};

#endif