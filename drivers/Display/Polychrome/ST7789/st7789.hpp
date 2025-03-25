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

    __fast_inline void write_command(const uint8_t cmd){
        interface.write_command(cmd);
    }

    __fast_inline void write_data(const uint8_t data){
        interface.write_data(data);
    }

    __fast_inline void write_data16(const uint16_t data){
        interface.write_data16(data);
    }

    void modify_ctrl(const bool yes,const uint8_t pos){
        uint8_t temp = 0x01 << pos;
        if (yes) scr_ctrl |= temp;
        else scr_ctrl &= ~temp;

        write_command(0x36);
        write_data(scr_ctrl);
    }



protected:

    __fast_inline uint32_t get_point_index(const uint16_t x, const uint16_t y){
        return (x + y * size_t(size().x));
    }

    void setpos_unsafe(const Vector2i & pos) override;
    void setarea_unsafe(const Rect2i & rect) override;

    __fast_inline void putpixel_unsafe(const Vector2i & pos, const RGB565 color){
        setpos_unsafe(pos);
        interface.write_data16(uint16_t(color));
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

    void put_texture(const Rect2i & rect, const is_color auto * color_ptr){
        setarea_unsafe(rect);
        interface.write_burst<RGB565>(color_ptr, int(rect));
    }

    void set_display_offset(const Vector2i & _offset){offset = _offset;}
    void set_flip_y(const bool flip){modify_ctrl(flip, 7);}
    void set_flip_x(const bool flip){modify_ctrl(flip, 6);}
    void set_swap_xy(const bool flip){modify_ctrl(flip, 5);}
    void set_flush_dir_v(const bool dir){modify_ctrl(dir, 4);}
    void set_format_rgb(const bool is_rgb){modify_ctrl(!is_rgb, 3);}
    void set_flush_dir_h(const bool dir){modify_ctrl(dir, 2);}

    void set_inversion(const bool inv){write_command(0x20 + inv);}
};

};