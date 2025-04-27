#pragma once


#include "image.hpp"

#include "core/string/StringView.hpp"
#include "core/stream/stream.hpp"
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
    using Cursor = Vector2i;

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
    Rect2i crop_rect;

    Font * enfont = nullptr;
    Font * chfont = nullptr;
    int padding = 1;

    [[nodiscard]] IResult<> drawHriLine(const Vector2i & pos,const int l){
        auto rect = Rect2i(pos, Vector2i(l, 1));
        // rect = this->getClipWindow.intersection(rect);
        if(bool(rect) == false) return Err(Error::AreaNotExist);
        return drawFilledRect(rect);
    }
    [[nodiscard]] IResult<> drawVerLine(const Vector2i & pos,const int l){
        auto rect = Rect2i(pos, Vector2i(1, l));
        // rect = this->getClipWindow.intersection(rect);
        if(bool(rect) == false) return Err(Error::AreaNotExist);
        return drawFilledRect(rect);
    }
    [[nodiscard]] IResult<> drawVerLine(const Rangei & y_range, const int x){
        auto y_range_regular = y_range.abs();
        return drawVerLine(Vector2i(x, y_range_regular.from), y_range_regular.length());
    }

    [[nodiscard]] IResult<> drawHriLine(const Rangei & x_range, const int y){
        auto x_range_regular = x_range.abs();
        return drawHriLine(Vector2i(x_range_regular.from, y), x_range_regular.length());
    }

    virtual IResult<> drawStr(const Vector2i & pos, const char * str_ptr, const size_t str_len) = 0;

    IResult<> draw_gbk_str(const Vector2i & pos, const StringView str){
        TODO();
        return Ok();
    }

    IResult<> draw_utf8_str(const Vector2i & pos, const StringView str){
        TODO();
        return Ok();
    }

public:
    DELETE_COPY_AND_MOVE(PainterConcept)

    PainterConcept() = default;
    [[nodiscard]] IResult<> fill(const RGB888 & color){
        this->setColor(color);
        drawFilledRect(this->getClipWindow());
        // return res;
        return Ok();
    }

    [[nodiscard]] IResult<> setChFont(Font & _chfont){
        chfont = &_chfont;
        return Ok();
    }

    [[nodiscard]] IResult<> setEnFont(Font & _enfont){
        enfont = &_enfont;
        return Ok();
    }


    void setColor(RGB888 _color){
        m_color = _color;
    }

    virtual void drawPixel(const Vector2i & pos) = 0;

    virtual IResult<> drawChar(const Vector2i & pos,const wchar_t chr) = 0;
    
    virtual Rect2i getClipWindow() = 0;
    
    virtual IResult<> drawLine(const Vector2i & from, const Vector2i & to) = 0;

    [[nodiscard]] IResult<> drawHollowRect(const Rect2i & rect);

    virtual IResult<> drawFilledRect(const Rect2i & rect) = 0;

    [[nodiscard]] IResult<> drawHollowCircle(const Vector2i & pos, const uint radius);

    [[nodiscard]] IResult<> drawFilledCircle(const Vector2i & pos, const uint radius);


    [[nodiscard]] IResult<> drawString(const Vector2i & pos, const StringView str);

    [[nodiscard]] IResult<> drawHollowEllipse(const Vector2i & pos, const Vector2i & r);

    [[nodiscard]] IResult<> drawFilledEllipse(const Vector2i & pos, const Vector2i & r);

    [[nodiscard]] IResult<> drawPolyline(std::span<const Vector2i> points);

    [[nodiscard]] IResult<> drawPolygon(std::span<const Vector2i> points);

    [[nodiscard]] IResult<> drawHollowTriangle(const Vector2i & p0,const Vector2i & p1,const Vector2i & p2);

    [[nodiscard]] IResult<> drawFilledTriangle(const Vector2i & p0,const Vector2i & p1,const Vector2i & p2);

    [[nodiscard]] IResult<> drawRoi(const Rect2i & rect);
};

}