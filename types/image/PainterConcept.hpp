#pragma once


#include "image.hpp"

#include "core/stream/stream.hpp"
#include "types/rect2/rect2.hpp"
#include "types/color/color_t.hpp"
#include "types/rgb.h"

#include "font/font.hpp"


namespace ymd{

class String;

class StringView;

class PainterConcept:public OutputStream{
protected:
    using Cursor = Vector2i;
    Cursor cursor = {0,0};

    RGB888 m_color;
    Rect2i crop_rect;

    Font * enfont = nullptr;
    Font * chfont = nullptr;
    int padding = 1;

    void drawHriLine(const Vector2i & pos,const int l){
        auto rect = Rect2i(pos, Vector2i(l, 1));
        // rect = this->getClipWindow.intersection(rect);
        if(bool(rect) == false) return;
        drawFilledRect(rect);
    }
    void drawVerLine(const Vector2i & pos,const int l){
        auto rect = Rect2i(pos, Vector2i(1, l));
        // rect = this->getClipWindow.intersection(rect);
        if(bool(rect) == false) return;
        drawFilledRect(rect);
    }
    void drawVerLine(const Rangei & y_range, const int x){
        auto y_range_regular = y_range.abs();
        drawVerLine(Vector2i(x, y_range_regular.from), y_range_regular.length());
    }

    void drawHriLine(const Rangei & x_range, const int y){
        auto x_range_regular = x_range.abs();
        drawHriLine(Vector2i(x_range_regular.from, y), x_range_regular.length());
    }

    virtual void drawStr(const Vector2i & pos, const char * str_ptr, const size_t str_len) = 0;

public:
    DELETE_COPY_AND_MOVE(PainterConcept)

    PainterConcept() = default;
    void write(const char data) override{
        drawChar(cursor, wchar_t(data));
    }

    void write(const char * data, const size_t len) override{
        drawStr(cursor, data, len);
    }

    size_t pending() const override{return 0;}

    void fill(const RGB888 & color){
        this->setColor(color);
        drawFilledRect(this->getClipWindow());
    }

    void setChFont(Font & _chfont){
        chfont = &_chfont;
    }

    void setEnFont(Font & _enfont){
        enfont = &_enfont;
    }

    template<typename U>
    void setColor(U _color){
        m_color = _color;
    }

    virtual void drawPixel(const Vector2i & pos) = 0;

    virtual void drawChar(const Vector2i & pos,const wchar_t chr) = 0;
    
    virtual Rect2i getClipWindow() = 0;
    
    virtual void drawLine(const Vector2i & from, const Vector2i & to) = 0;

    void drawHollowRect(const Rect2i & rect);

    virtual void drawFilledRect(const Rect2i & rect) = 0;

    void drawHollowCircle(const Vector2i & pos, const uint radius);

    void drawFilledCircle(const Vector2i & pos, const uint radius);

    void drawString(const Vector2i & pos, const String & str);

    void drawString(const Vector2i & pos, const StringView & str);

    void drawString(const Vector2i & pos, const char * str);


    void drawHollowEllipse(const Vector2i & pos, const Vector2i & r);

    void drawFilledEllipse(const Vector2i & pos, const Vector2i & r);

    void drawPolyline(const Vector2i * points, const size_t count);

    void drawPolygon(const Vector2i * points, const size_t count);

    void drawPolyline(const std::initializer_list<Vector2i> & points);

    void drawPolygon(const std::initializer_list<Vector2i> & points);


    void drawHollowTriangle(const Vector2i & p0,const Vector2i & p1,const Vector2i & p2);

    void drawFilledTriangle(const Vector2i & p0,const Vector2i & p1,const Vector2i & p2);

    void drawRoi(const Rect2i & rect);
};

}