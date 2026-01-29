#pragma once

#include "ssd1306_prelude.hpp"

namespace ymd::drivers{

class SSD13XX final:
    public SSD13XX_Prelude
{
public:


    template<typename Cfg, typename T = std::decay_t<Cfg>>
    explicit SSD13XX(Phy && phy, Cfg && cfg):
        transport_(std::move(phy)),
        frame_(VerticalBinaryImage{cfg.size}){;}


    template<typename Cfg>
    [[nodiscard]] IResult<> init(Cfg && cfg){
            // DEBUG_PRINTLN(std::showbase, std::hex, init_cmds_list_);
        if(const auto res = transport_.init() ; 
            res.is_err()) return res;
        if(const auto res = preinit_by_cmds(cfg.buf); 
            res.is_err()) return res;
        if(const auto res = enable_display(EN); 
            res.is_err()) return res;
        if(const auto res = set_offset(cfg.offset);
            res.is_err()) return res;
        if(const auto res = enable_flip_x(DISEN); 
            res.is_err()) return res;
        if(const auto res = enable_flip_y(DISEN); 
            res.is_err()) return res;
        return Ok();
    }

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> enable_display(const Enable en);
    [[nodiscard]] IResult<> enable_flip_y(const Enable flip_en = EN){
        return transport_.write_command(0xA0 | (flip_en == EN));}
    [[nodiscard]] IResult<> enable_flip_x(const Enable flip_en = EN){
        return transport_.write_command(0xC0 | ((flip_en == EN) << 3));}
    [[nodiscard]] IResult<> enable_inversion(const Enable inv_en = EN){
        return transport_.write_command(0xA7 - (inv_en == EN));}  

    [[nodiscard]] math::Vec2<uint16_t> size() const {return frame_.size();}
    VerticalBinaryImage & fetch_frame() {return frame_;};

private:


    Phy transport_;

    const math::Vec2<uint16_t> offset_;
    VerticalBinaryImage frame_;

    [[nodiscard]] IResult<> put_pixel_unchecked(const math::Vec2<uint16_t> pos, const Binary color){
        auto & frame = fetch_frame();
        frame.put_pixel_unchecked(pos, color);
        return Ok();
    }

    [[nodiscard]] IResult<> putrect_unchecked(const math::Rect2u16 rect, const Binary color){
        auto & frame = fetch_frame();
        frame.put_pixel_unchecked(rect.top_left, color);
        return Ok();
    }

    [[nodiscard]] IResult<> put_texture_unchecked(const math::Rect2u16 rect, const Binary * pcolor){
        auto & frame = fetch_frame();
        frame.put_pixel_unchecked(rect.top_left, pcolor[0]);
        return Ok();
    }

    [[nodiscard]] IResult<> setpos_unchecked(const math::Vec2<uint16_t> pos) ;

    [[nodiscard]] IResult<> set_offset(const math::Vec2<uint16_t> offset);

    [[nodiscard]] IResult<> set_flush_pos(const math::Vec2<uint16_t> pos);

    [[nodiscard]] IResult<> preinit_by_cmds(const std::span<const uint8_t> pbuf);

    IResult<> write_command(const uint8_t cmd){
        return transport_.write_command(cmd);
    }

    template<typename T>
    friend class DrawTarget;
};


template<>
class DrawTarget<SSD13XX>{
public:
    static constexpr bool IS_BUFFERED = true;

    using Error = DisplayerError;
    template<typename T = void>
    using IResult = Result<T, Error>;

    [[nodiscard]] static IResult<> put_pixel_unchecked(
        SSD13XX & self, const math::Vec2<uint16_t> pos, const Binary color){
        return self.put_pixel_unchecked(pos, color);
    }

    [[nodiscard]] static IResult<> putrect_unchecked(
        SSD13XX & self, const math::Rect2u16 rect, const Binary color){
        return self.putrect_unchecked(rect, color);
    }

    [[nodiscard]] static IResult<> put_texture_unchecked(
        SSD13XX & self, const math::Rect2u16 rect, const Binary * pcolor){
        return self.put_texture_unchecked(rect, pcolor);
    }


};

}
