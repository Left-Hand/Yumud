#pragma once


#include "image.hpp"

#include "core/string/StringView.hpp"
#include "core/stream/stream.hpp"
#include "core/stream/StringStream.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"


#include "types/rect2/rect2.hpp"
#include "types/color/color_t.hpp"
#include "types/rgb.h"

#include "font/font.hpp"

namespace ymd{

class String;

class StringView;

class PainterConcept{
protected:
    using Cursor = Vector2u;

    enum class Error_Kind{
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
    };

    DEF_ERROR_WITH_KIND(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    Cursor cursor = {0,0};

    RGB888 m_color;
    Rect2u crop_rect;

    Font * enfont = nullptr;
    Font * chfont = nullptr;
    int padding = 1;

    [[nodiscard]] IResult<> draw_hri_line(const Vector2u & pos,const int l){
        auto ins = Rect2u(pos, Vector2u(l, 1));
        // ins = this->getClipWindow.intersection(ins);
        if(ins.get_area() == 0) return Err(Error::AreaNotExist);
        return draw_filled_rect(ins);
    }
    [[nodiscard]] IResult<> draw_ver_line(const Vector2u & pos,const int l){
        auto ins = Rect2u(pos, Vector2u(1, l));
        // ins = this->getClipWindow.intersection(ins);
        if(ins.get_area() == 0) return Err(Error::AreaNotExist);
        return draw_filled_rect(ins);
    }
    [[nodiscard]] IResult<> draw_ver_line(const Range2i & y_range, const int x){
        auto y_range_regular = y_range.abs();
        return draw_ver_line(Vector2u(x, y_range_regular.from), y_range_regular.length());
    }

    [[nodiscard]] IResult<> draw_hri_line(const Range2i & x_range, const int y){
        auto x_range_regular = x_range.abs();
        return draw_hri_line(Vector2u(x_range_regular.from, y), x_range_regular.length());
    }

    virtual IResult<> draw_str(const Vector2u & pos, const char * str_ptr, const size_t str_len) = 0;

    IResult<> draw_gbk_str(const Vector2u & pos, const StringView str){
        TODO();
        return Ok();
    }

    IResult<> draw_utf8_str(const Vector2u & pos, const StringView str){
        TODO();
        return Ok();
    }

public:
    DELETE_COPY_AND_MOVE(PainterConcept)

    PainterConcept() = default;
    [[nodiscard]] IResult<> fill(const RGB888 & color){
        this->set_color(color);
        draw_filled_rect(this->get_clip_window());
        // return res;
        return Ok();
    }

    [[nodiscard]] IResult<> set_ch_font(Font & _chfont){
        chfont = &_chfont;
        return Ok();
    }

    [[nodiscard]] IResult<> set_en_font(Font & _enfont){
        enfont = &_enfont;
        return Ok();
    }


    void set_color(RGB888 _color){
        m_color = _color;
    }

    virtual void draw_pixel(const Vector2u & pos) = 0;

    virtual IResult<> draw_char(const Vector2u & pos,const wchar_t chr) = 0;
    
    virtual Rect2u get_clip_window() = 0;
    
    virtual IResult<> draw_line(const Vector2u & from, const Vector2u & to) = 0;

    [[nodiscard]] IResult<> draw_hollow_rect(const Rect2u & rect);

    virtual IResult<> draw_filled_rect(const Rect2u & rect) = 0;

    [[nodiscard]] IResult<> draw_hollow_circle(const Vector2u & pos, const uint radius);

    [[nodiscard]] IResult<> draw_filled_circle(const Vector2u & pos, const uint radius);


    [[nodiscard]] IResult<> draw_string(const Vector2u & pos, const StringView str);

    [[nodiscard]] IResult<> draw_hollow_ellipse(const Vector2u & pos, const Vector2u & r);

    [[nodiscard]] IResult<> draw_filled_ellipse(const Vector2u & pos, const Vector2u & r);

    [[nodiscard]] IResult<> draw_polyline(std::span<const Vector2u> points);

    [[nodiscard]] IResult<> draw_polygon(std::span<const Vector2u> points);

    [[nodiscard]] IResult<> draw_hollow_triangle(const Vector2u & p0,const Vector2u & p1,const Vector2u & p2);

    [[nodiscard]] IResult<> draw_filled_triangle(const Vector2u & p0,const Vector2u & p1,const Vector2u & p2);

    [[nodiscard]] IResult<> draw_roi(const Rect2u & rect);

    template<typename ... Ts>
    [[nodiscard]] IResult<> draw_args(const Vector2u pos, Ts && ... args){
        StringStream ss;
        ss.print(std::forward<Ts>(args)...);
        return draw_string(pos, StringView(std::move(ss).move_str()));
    }
    
    template<typename Fn>
    [[nodiscard]]
    IResult<> draw_fx(const Rect2u rect, Fn && fn) {
        const auto x_range = rect.get_x_range();
        const auto y_range = rect.get_y_range();
        const auto x_step = real_t(1) / x_range.length();

        real_t x = 0;
        for(size_t i = size_t(x_range.from); i < size_t(x_range.to); i++){
            const auto y = std::forward<Fn>(fn)(x);
            draw_pixel({uint(i),uint(
                y * (y_range.length()) + y_range.from
            )});
            x = x + x_step;
        }

        return Ok();
    }
};

}