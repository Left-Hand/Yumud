#pragma once

#include "st7789_phy.hpp"
#include "st7789v3_phy.hpp"

namespace ymd::drivers{
class ST7789 final:
    public ST7789_Prelude{
public:
    explicit ST7789(
        ST7789_Phy && phy, 
        const Vector2<uint16_t> size
    ):
        phy_(phy),
        algo_(size){;}


    auto size() const {return algo_.size();}

    IResult<> init();
    IResult<> fill(const RGB565 color){
        return putrect_unchecked(size().to_rect(), color);
    }
    IResult<> setpos_unchecked(const Vector2<uint16_t> pos);
    IResult<> setarea_unchecked(const Rect2<uint16_t> rect);

    IResult<> put_texture(const Rect2<uint16_t> & rect, const is_color auto * pcolor){
        if(const auto res = setarea_unchecked(rect);
            res.is_err()) return res;
        if(const auto res = put_next_texture(rect, pcolor);
            res.is_err()) return res;
        return Ok();
    }

    IResult<> put_next_texture(const Rect2<uint16_t> rect, const is_color auto * pcolor){
        return phy_.write_burst_pixels(std::span(pcolor, rect.get_area()));
    }

    IResult<> set_display_offset(const Vector2<uint16_t> _offset){
        offset_ = _offset;
        return Ok();
    }
    
    IResult<> enable_flip_y(const Enable flip){
        return modify_ctrl_reg(flip == EN, 7);
    }

    IResult<> enable_flip_x(const Enable flip){
        return modify_ctrl_reg(flip == EN, 6);
    }

    IResult<> enable_swap_xy(const Enable flip){
        return modify_ctrl_reg(flip == EN, 5);
    }

    IResult<> set_flush_dir_v(const bool dir){
        return modify_ctrl_reg(dir, 4);
    }

    IResult<> set_format_rgb(const bool is_rgb){
        return modify_ctrl_reg(!is_rgb, 3);
    }

    IResult<> set_flush_dir_h(const bool dir){
        return modify_ctrl_reg(dir, 2);
    }


    IResult<> enable_inversion(const Enable inv_en){
        return write_command((inv_en == EN) ? 0x21 : 0x20);
    }

    [[nodiscard]] __fast_inline IResult<> putpixel_unchecked(
        const Vector2<uint16_t> pos, 
        const RGB565 color
    ){
        if(const auto res = setpos_unchecked(pos);
            res.is_err()) return res;
        if(const auto res = phy_.write_data16(color.as_u16());
            res.is_err()) return res;
        return Ok();
    }

    [[nodiscard]] IResult<> putrect_unchecked(
        const Rect2<uint16_t> rect, 
        const RGB565 color
    );

    [[nodiscard]] IResult<> puttexture_unchecked(
        const Rect2<uint16_t> rect, 
        const RGB565 * pcolor
    );

    [[nodiscard]] Rect2u get_expose_rect(){
        return algo_.size().to_rect();
    }

private:
    using Algo = ST7789_ReflashAlgo;

    ST7789_Phy phy_;
    Algo algo_;

    Vector2<uint16_t> offset_;
    uint8_t scr_ctrl_ = 0;

    [[nodiscard]] __fast_inline IResult<> write_command(const uint8_t cmd){
        return phy_.write_command(cmd);
    }

    [[nodiscard]] __fast_inline IResult<> write_data8(const uint8_t data){
        return phy_.write_data8(data);
    }

    [[nodiscard]] __fast_inline IResult<> write_data16(const uint16_t data){
        return phy_.write_data16(data);
    }

    [[nodiscard]] IResult<> modify_ctrl_reg(const bool is_high, const uint8_t pos);

};


namespace st7789_preset{
    struct _120X80{};
    struct _240X135{};
    struct _320X170{};

    template<typename T>
    struct ST7789_InitDispatcher{
    };

    template<>
    struct ST7789_InitDispatcher<_120X80>{

        static ST7789_Prelude::IResult<void> init(ST7789 & displayer, _120X80){
            if(const auto res = displayer.enable_flip_x(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flip_y(EN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_swap_xy(EN);
                res.is_err()) return res;
            if(const auto res = displayer.set_display_offset({40, 52}); 
                res.is_err()) return res;
            if(const auto res = displayer.set_format_rgb(true);
                res.is_err()) return res;
            if(const auto res = displayer.set_flush_dir_h(false);
                res.is_err()) return res;
            if(const auto res = displayer.set_flush_dir_v(false);
                res.is_err()) return res;
            if(const auto res = displayer.enable_inversion(EN);
                res.is_err()) return res;
            return Ok();
        }
    };

    template<>
    struct ST7789_InitDispatcher<_240X135>{

        static ST7789_Prelude::IResult<void> init(ST7789 & displayer, _240X135){
            if(const auto res = displayer.enable_flip_x(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flip_y(EN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_swap_xy(EN);
                res.is_err()) return res;
            if(const auto res = displayer.set_display_offset({40, 52}); 
                res.is_err()) return res;
            if(const auto res = displayer.set_format_rgb(true);
                res.is_err()) return res;
            if(const auto res = displayer.set_flush_dir_h(false);
                res.is_err()) return res;
            if(const auto res = displayer.set_flush_dir_v(false);
                res.is_err()) return res;
            if(const auto res = displayer.enable_inversion(EN);
                res.is_err()) return res;
            return Ok();
        }
    };

    template<>
    struct ST7789_InitDispatcher<_320X170> {
        
        static ST7789_Prelude::IResult<void> init(ST7789 & displayer, _320X170){

            if(const auto res = displayer.enable_flip_x(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flip_y(EN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_swap_xy(EN);
                res.is_err()) return res;
            if(const auto res = displayer.set_display_offset({0, 35}); 
                res.is_err()) return res;
            if(const auto res = displayer.set_format_rgb(true);
                res.is_err()) return res;
            if(const auto res = displayer.set_flush_dir_h(false);
                res.is_err()) return res;
            if(const auto res = displayer.set_flush_dir_v(false);
                res.is_err()) return res;
            if(const auto res = displayer.enable_inversion(EN);
                res.is_err()) return res;
            return Ok();
        }
    };


    template<typename T>
    ST7789_Prelude::IResult<void> init(ST7789 & displayer, T && preset){
        return ST7789_InitDispatcher<T>::init(displayer, std::forward<T>(preset));
    }
}


};