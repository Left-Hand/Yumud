#pragma once

#include "drivers/Display/DisplayerPhy.hpp"
#include "types/image/image.hpp"
#include "hal/bus/spi/spi.hpp"

namespace ymd::drivers{

class ST7789:public Displayer<RGB565>{
public:
    class ST7789_ReflashAlgo{
    public:

        ST7789_ReflashAlgo(const Vector2_t<uint16_t> & size):
            size_(size){;}

        __fast_inline constexpr
        uint32_t get_point_index(const Vector2_t<uint16_t> p){
            return (p.x + p.y * size_t(size_.x));
        }

        __fast_inline constexpr
        Range_t<uint32_t> get_point_index(const Rect2_t<uint16_t> r){
            return {
                get_point_index(r.position), 
                get_point_index({r.position.x + r.size.x - 1, r.position.y + r.size.y - 1})};
        }

        bool update(const Rect2_t<uint16_t> rect);
        bool update(const Vector2_t<uint16_t> p){
            return update(Rect2_t<uint16_t>{p, size_});
        }
    private:
        const Vector2_t<uint16_t> size_;
        Rect2_t<uint16_t> curr_area_ = Rect2i(0,0,1,1);
        uint32_t last_point_ = 0;
    };

private:
    using Algo = ST7789_ReflashAlgo;

    DisplayerPhySpi interface_;
    Algo algo_;

    Vector2_t<uint16_t> offset_;
    uint8_t scr_ctrl_ = 0;

    __fast_inline void write_command(const uint8_t cmd){
        interface_.write_command(cmd);
    }

    __fast_inline void write_data(const uint8_t data){
        interface_.write_data(data);
    }

    __fast_inline void write_data16(const uint16_t data){
        interface_.write_data16(data);
    }

    void modify_ctrl(const bool yes,const uint8_t pos){
        uint8_t temp = 0x01 << pos;
        if (yes) scr_ctrl_ |= temp;
        else scr_ctrl_ &= ~temp;

        write_command(0x36);
        write_data(scr_ctrl_);
    }

protected:

    void setpos_unsafe(const Vector2i & pos);
    void setarea_unsafe(const Rect2i & rect);

    __fast_inline void putpixel_unsafe(const Vector2i & pos, const RGB565 color){
        setpos_unsafe(pos);
        interface_.write_data16(uint16_t(color));
    }

    void putrect_unsafe(const Rect2i & rect, const RGB565 color);
    void puttexture_unsafe(const Rect2i & rect, const RGB565 * color_ptr);
    void putseg_v8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color);
    void putseg_h8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color);
public:
    ST7789(const DisplayerPhySpi & interface, const Vector2_t<uint16_t> & size):
            ImageBasics(size), 
            Displayer<RGB565>(size), 
            interface_(interface),
            algo_(size){;}

    void init();

    void put_texture(const Rect2i & rect, const is_color auto * color_ptr){
        setarea_unsafe(rect);
        interface_.write_burst<RGB565>(color_ptr, int(rect));
    }

    void set_display_offset(const Vector2i & _offset){offset_ = _offset;}
    void set_flip_y(const bool flip){modify_ctrl(flip, 7);}
    void set_flip_x(const bool flip){modify_ctrl(flip, 6);}
    void set_swap_xy(const bool flip){modify_ctrl(flip, 5);}
    void set_flush_dir_v(const bool dir){modify_ctrl(dir, 4);}
    void set_format_rgb(const bool is_rgb){modify_ctrl(!is_rgb, 3);}
    void set_flush_dir_h(const bool dir){modify_ctrl(dir, 2);}

    void set_inversion(const bool inv){write_command(0x20 + inv);}
};

};