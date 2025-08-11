#pragma once


#include "types/image/image.hpp"

#include "core/string/string_view.hpp"
#include "core/stream/stream.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/scope_guard.hpp"

#include "types/regions/rect2/rect2.hpp"
#include "types/colors/color/color.hpp"
#include "types/colors/rgb/rgb.hpp"

#include "types/image/font/font.hpp"

namespace ymd{

class String;

class StringView;


struct PainterPrelude{
    using Cursor = Vector2u;

    enum class Error_Kind:uint8_t{
        ImageNotSet = 1,
        AreaNotExist,
        OutOfBound,
        StartPointOutOfBound,
        EndPointOutOfBound,
        MinusRadius,
        NoFontFounded,  
        NoEnglishFontFounded,
        NoChineseFontFounded,
        StringLengthTooLong,
        PointsTooLess,
        Unfinished,
        CropRectIsNone
    };

    DEF_ERROR_WITH_KIND(Error, Error_Kind)
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

class PainterDispatcherIntf:public PainterPrelude{
public:
    [[nodiscard]] virtual IResult<> draw_wchar(const Vector2u & pos,const wchar_t chr) = 0;
    
    [[nodiscard]] virtual Option<Rect2u> get_expose_rect() = 0;
    
    [[nodiscard]] virtual IResult<> draw_line(const Vector2u & start, const Vector2u & stop) = 0;

    [[nodiscard]] virtual IResult<> draw_c_str(const Vector2u & pos, const StringView str) = 0;

    [[nodiscard]] virtual IResult<> draw_filled_rect(const Rect2u & rect) = 0;
};

class PainterBase:public PainterPrelude{
public:
    PainterBase(const PainterBase & other) = delete;
    PainterBase(PainterBase && other) = delete;

    PainterBase() = default;
    [[nodiscard]] IResult<> fill(const RGB888 & color){
        const auto orginal_color = get_color();
        auto guard = make_scope_guard([&]{
            set_color(orginal_color);
        });

        this->set_color(color);
        const auto may_rect = this->get_expose_rect();
        if(may_rect.is_none()) return Err(Error::CropRectIsNone);
        if(const auto res = draw_filled_rect(may_rect.unwrap());
            res.is_err()) return res;
        return Ok();
    }

    [[nodiscard]] IResult<> set_ch_font(Some<Font *> chfont){
        may_chfont_ = chfont;
        return Ok();
    }

    [[nodiscard]] IResult<> set_en_font(Some<Font *> enfont){
        may_enfont_ = enfont;
        return Ok();
    }
    [[nodiscard]] auto create_color_guard(){
        struct ColorGuard{
            PainterBase & owner;
            RGB888 color;

            ~ColorGuard(){
                owner.set_color(color);
            }
        };

        return ColorGuard{*this, color_};
    }

    template<typename T>
    void set_color(const T color){
        color_ = static_cast<RGB888>(color);
    }

    RGB888 get_color() const{
        return color_;
    }

    [[nodiscard]] IResult<> draw_hollow_rect(const Rect2u & rect);

    [[nodiscard]] IResult<> draw_hollow_circle(const Vector2u & pos, const uint radius);

    [[nodiscard]] IResult<> draw_filled_circle(const Vector2u & pos, const uint radius);


    [[nodiscard]] IResult<> draw_hollow_ellipse(const Vector2u & pos, const Vector2u & r);

    [[nodiscard]] IResult<> draw_filled_ellipse(const Vector2u & pos, const Vector2u & r);

    [[nodiscard]] IResult<> draw_polyline(std::span<const Vector2u> points);

    [[nodiscard]] IResult<> draw_polygon(std::span<const Vector2u> points);

    [[nodiscard]] IResult<> draw_hollow_triangle(const Vector2u & p0,const Vector2u & p1,const Vector2u & p2);

    [[nodiscard]] IResult<> draw_filled_triangle(const Vector2u & p0,const Vector2u & p1,const Vector2u & p2);

    [[nodiscard]] IResult<> draw_roi(const Rect2u & rect);

    #if 0
    template<typename ... Ts>
    [[nodiscard]] IResult<> draw_args(const Vector2u pos, Ts && ... args){
        StringStream ss;
        ss.print(std::forward<Ts>(args)...);
        return draw_string(pos, StringView(std::move(ss).move_str()));
    }
    #endif
    
    template<typename Fn>
    [[nodiscard]]
    IResult<> draw_fx(const Rect2u rect, Fn && fn) {
        const auto x_range = rect.get_x_range();
        const auto y_range = rect.get_y_range();
        const auto x_step = real_t(1) / x_range.length();

        real_t x = 0;
        for(size_t i = size_t(x_range.start); i < size_t(x_range.stop); i++){
            const auto y = std::forward<Fn>(fn)(x);
            putpixel_unchecked({uint(i),uint(
                y * (y_range.length()) + y_range.start
            )});
            x = x + x_step;
        }

        return Ok();
    }

    virtual void putpixel_unchecked(const Vector2<uint16_t> pos);

    [[nodiscard]] virtual IResult<> draw_wchar(const Vector2u & pos,const wchar_t chr) = 0;

    [[nodiscard]] virtual IResult<> draw_gbk_str(const Vector2u & pos, const StringView str) = 0;

    [[nodiscard]] virtual IResult<> draw_ascii_str(const Vector2u & pos, const StringView str) = 0;

    [[nodiscard]] virtual Option<Rect2u> get_expose_rect() {return None;}
    
    [[nodiscard]] virtual IResult<> draw_line(const Vector2u & start, const Vector2u & stop) = 0;



    [[nodiscard]] virtual IResult<> draw_filled_rect(const Rect2u & rect) = 0;

    [[nodiscard]] IResult<> set_crop_rect(const Rect2u & rect){
        crop_rect_ = rect;
        return Ok();
    }

    [[nodiscard]] Rect2u get_crop_rect() const{
        return crop_rect_;
    }
protected:
    Cursor cursor_ = {0,0};

    RGB888 color_;
    Rect2u crop_rect_;

    Option<Font &> may_enfont_ = None;
    Option<Font &> may_chfont_ = None;
    size_t padding_ = 1;

    [[nodiscard]] IResult<> draw_hri_line(const Vector2u & pos,const int l);

    [[nodiscard]] IResult<> draw_ver_line(const Vector2u & pos,const int l);

    [[nodiscard]] IResult<> draw_ver_line(const Range2u & y_range, const int x);

    [[nodiscard]] IResult<> draw_hri_line(const Range2u & x_range, const int y);
};

}