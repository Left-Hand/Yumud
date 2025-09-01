#pragma once

#include "st7789_phy.hpp"
// #include "st7789v3_phy.hpp"

namespace ymd::drivers{

class ST7789 final:
    public ST7789_Prelude{
public:
    explicit ST7789(
        ST7789_Phy && phy, 
        const Vec2<uint16_t> size
    ):
        phy_(phy),
        algo_(size){;}


    Vec2<uint16_t> size() const {return algo_.size();}

    Rect2u16 bounding_box() const {
        return Rect2u16{
            Vec2<uint16_t>::ZERO,
            size()
        };
    }

    template<typename Configer>
    IResult<> init(Configer && cfger){
        if(const auto res = this->common_init();
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = cfger.advanced_init(*this);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
    IResult<> fill(const RGB565 color){
        return putrect_unchecked(Rect2u16::from_size(size()), color);
    }
    IResult<> setpos_unchecked(const Vec2<uint16_t> pos);
    IResult<> setarea_unchecked(const Rect2<uint16_t> rect);

    IResult<> put_texture(const Rect2<uint16_t> & rect, const is_color auto * pcolor){
        if(const auto res = setarea_unchecked(rect);
            res.is_err()) return res;
        if(const auto res = put_next_texture(rect, pcolor);
            res.is_err()) return res;
        return Ok();
    }

    IResult<> put_next_texture(const Rect2<uint16_t> rect, const is_color auto * pcolor){
        return phy_.write_burst_pixels(std::span(pcolor, rect.area()));
    }

    IResult<> set_display_offset(const Vec2<uint16_t> _offset){
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

    IResult<> enable_flush_dir_v(const Enable dir){
        return modify_ctrl_reg(dir == EN, 4);
    }

    IResult<> enable_format_rgb(const Enable is_rgb){
        return modify_ctrl_reg(is_rgb == DISEN, 3);
    }

    IResult<> enable_flush_dir_h(const Enable dir){
        return modify_ctrl_reg(dir == EN, 2);
    }
    IResult<> enable_inversion(const Enable inv_en){
        return write_command((inv_en == EN) ? 0x21 : 0x20);
    }

    [[nodiscard]] __fast_inline IResult<> putpixel_unchecked(
        const Vec2<uint16_t> pos, 
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
        return Rect2u::from_size(algo_.size());
    }

private:
    using Algo = ST7789_ReflashAlgo;

    ST7789_Phy phy_;
    Algo algo_;

    Vec2<uint16_t> offset_ = Vec2<uint16_t>::ZERO;
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

    [[nodiscard]] IResult<> common_init();
    template<typename T>
    friend class DrawTarget;
};


template<>
struct DrawTarget<ST7789>{
    [[nodiscard]] static auto putpixel_unchecked(
        ST7789 & self,
        const Vec2<uint16_t> pos, 
        const RGB565 color
    ){
        return self.putpixel_unchecked(pos, color);
    }

    [[nodiscard]] static auto putrect_unchecked(
        ST7789 & self,
        const Rect2<uint16_t> rect, 
        const RGB565 color
    ){
        return self.putrect_unchecked(rect, color);
    }

    [[nodiscard]] static auto puttexture_unchecked(
        ST7789 & self,
        const Rect2<uint16_t> rect, 
        const RGB565 * pcolor
    ){
        return self.puttexture_unchecked(rect, pcolor);
    }
};


namespace st7789_preset{
    struct _120X80{
        static ST7789_Prelude::IResult<void> advanced_init(ST7789 & displayer){
            if(const auto res = displayer.enable_flip_x(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flip_y(EN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_swap_xy(EN);
                res.is_err()) return res;
            if(const auto res = displayer.set_display_offset({40, 52}); 
                res.is_err()) return res;
            if(const auto res = displayer.enable_format_rgb(EN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flush_dir_h(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flush_dir_v(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_inversion(EN);
                res.is_err()) return res;
            return Ok();
        }
    };
    struct _240X135{
        static ST7789_Prelude::IResult<void> advanced_init(ST7789 & displayer){
            if(const auto res = displayer.enable_flip_x(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flip_y(EN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_swap_xy(EN);
                res.is_err()) return res;
            if(const auto res = displayer.set_display_offset({40, 52}); 
                res.is_err()) return res;
            if(const auto res = displayer.enable_format_rgb(EN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flush_dir_h(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_flush_dir_v(DISEN);
                res.is_err()) return res;
            if(const auto res = displayer.enable_inversion(EN);
                res.is_err()) return res;
            return Ok();
        }
    };
    struct _320X170{
        static ST7789_Prelude::IResult<void> advanced_init(ST7789 & displayer);
    };

}


};