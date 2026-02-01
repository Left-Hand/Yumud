#pragma once

#include "st7789_phy.hpp"
// #include "st7789v3_phy.hpp"

namespace ymd::drivers{

class ST7789 final:
    public ST7789_Prelude{
public:
    explicit ST7789(
        ST7789_Transport && phy, 
        const math::Vec2<uint16_t> size
    ):
        transport_(phy),
        algo_(size){;}


    math::Vec2<uint16_t> size() const {return algo_.size();}

    math::Rect2u16 bounding_box() const {
        return math::Rect2u16{
            math::Vec2<uint16_t>::ZERO,
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
        return putrect_unchecked(math::Rect2u16::from_size(size()), color);
    }
    IResult<> setpos_unchecked(const math::Vec2<uint16_t> pos);
    IResult<> setarea_unchecked(const math::Rect2<uint16_t> rect);

    IResult<> put_texture(const math::Rect2<uint16_t> & rect, const is_color auto * pcolor){
        if(const auto res = setarea_unchecked(rect);
            res.is_err()) return res;
        if(const auto res = put_next_texture(rect, pcolor);
            res.is_err()) return res;
        return Ok();
    }

    IResult<> put_next_texture(const math::Rect2<uint16_t> rect, const is_color auto * pcolor){
        return transport_.write_burst_pixels(std::span(pcolor, rect.area()));
    }

    IResult<> set_display_offset(const math::Vec2<uint16_t> _offset){
        offset_ = _offset;
        return Ok();
    }
    
    IResult<> enable_flip_y(const Enable en){
        return modify_ctrl_reg(en == EN, 7);
    }

    IResult<> enable_flip_x(const Enable en){
        return modify_ctrl_reg(en == EN, 6);
    }

    IResult<> enable_swap_xy(const Enable en){
        return modify_ctrl_reg(en == EN, 5);
    }

    IResult<> enable_flush_dir_v(const Enable en){
        return modify_ctrl_reg(en == EN, 4);
    }

    IResult<> enable_format_rgb(const Enable en){
        return modify_ctrl_reg(en == DISEN, 3);
    }

    IResult<> enable_flush_dir_h(const Enable en){
        return modify_ctrl_reg(en == EN, 2);
    }
    IResult<> enable_inversion(const Enable inv_en){
        return write_command((inv_en == EN) ? 0x21 : 0x20);
    }

    [[nodiscard]] __fast_inline IResult<> put_pixel_unchecked(
        const math::Vec2<uint16_t> pos, 
        const RGB565 color
    ){
        if(const auto res = setpos_unchecked(pos);
            res.is_err()) return res;
        if(const auto res = transport_.write_data16(color.to_u16());
            res.is_err()) return res;
        return Ok();
    }

    [[nodiscard]] IResult<> putrect_unchecked(
        const math::Rect2<uint16_t> rect, 
        const RGB565 color
    );

    [[nodiscard]] IResult<> put_texture_unchecked(
        const math::Rect2<uint16_t> rect, 
        const RGB565 * pcolor
    );

    [[nodiscard]] math::Rect2u get_expose_rect(){
        return math::Rect2u::from_size(algo_.size());
    }

private:
    using Algo = ST7789_ReflashAlgo;

    ST7789_Transport transport_;
    Algo algo_;

    math::Vec2<uint16_t> offset_ = math::Vec2<uint16_t>::ZERO;
    uint8_t scr_ctrl_ = 0;

    [[nodiscard]] __fast_inline IResult<> write_command(const uint8_t cmd){
        return transport_.write_command(cmd);
    }

    [[nodiscard]] __fast_inline IResult<> write_data8(const uint8_t data){
        return transport_.write_data8(data);
    }

    [[nodiscard]] __fast_inline IResult<> write_data16(const uint16_t data){
        return transport_.write_data16(data);
    }

    [[nodiscard]] IResult<> modify_ctrl_reg(const bool is_high, const uint8_t pos);

    [[nodiscard]] IResult<> common_init();
    template<typename T>
    friend class DrawTarget;
};


template<>
struct DrawTarget<ST7789>{
    [[nodiscard]] static auto put_pixel_unchecked(
        ST7789 & self,
        const math::Vec2<uint16_t> pos, 
        const RGB565 color
    ){
        return self.put_pixel_unchecked(pos, color);
    }

    [[nodiscard]] static auto putrect_unchecked(
        ST7789 & self,
        const math::Rect2<uint16_t> rect, 
        const RGB565 color
    ){
        return self.putrect_unchecked(rect, color);
    }

    [[nodiscard]] static auto put_texture_unchecked(
        ST7789 & self,
        const math::Rect2<uint16_t> rect, 
        const RGB565 * pcolor
    ){
        return self.put_texture_unchecked(rect, pcolor);
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