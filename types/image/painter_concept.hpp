#pragma once

#include "sys/string/string.hpp"
#include "sys/string/string_view.hpp"
#include "sys/debug/debug_inc.h"

#include "image.hpp"
#include "packed_image.hpp"

#include "types/rect2/rect2_t.hpp"
#include "types/color/color_t.hpp"
#include "types/rgb.h"

#include "font/font.hpp"


namespace yumud{

class PainterConcept{
protected:

    RGB888 m_color;
    Rect2i crop_rect;
    
    void drawHriLine(const Vector2i & pos,const int l){
        auto rect = Rect2i(pos, Vector2i(l, 1));
        // rect = this->getCanvasWindow().intersection(rect);
        if(bool(rect) == false) return;
        drawFilledRect(rect);
    }
    void drawVerLine(const Vector2i & pos,const int l){
        auto rect = Rect2i(pos, Vector2i(1, l));
        // rect = this->getCanvasWindow().intersection(rect);
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

    template<typename U>
    void setColor(U _color){
        m_color = _color;
    }

    virtual void drawPixel(const Vector2i & pos) = 0;

    virtual void drawChar(const Vector2i & pos,const wchar_t chr) = 0;
    
    virtual Rect2i getCanvasWindow() = 0;
    
    virtual Vector2i getCanvasSize() {return getCanvasWindow().size;}
    
    virtual void drawLine(const Vector2i & from, const Vector2i & to) = 0;

    void drawHollowRect(const Rect2i & rect);

    virtual void drawFilledRect(const Rect2i & rect) = 0;

    void drawHollowCircle(const Vector2i & pos, const uint radius);

    void drawFilledCircle(const Vector2i & pos, const uint radius);

    void drawString(const Vector2i & pos, const String & str);

    void drawString(const Vector2i & pos, const StringView str);

    void drawString(const Vector2i & pos, const char * str);


    void drawHollowEllipse(const Vector2i & pos, const Vector2i & r);

    void drawFilledEllipse(const Vector2i & pos, const Vector2i & r);

    void drawPolyline(const Vector2i * points, const size_t count);

    void drawPolygon(const Vector2i * points, const size_t count);

    void drawPolyline(const std::initializer_list<Vector2i> & points){
        drawPolyline(points.begin(), points.size());
    }

    void drawPolygon(const std::initializer_list<Vector2i> & points){
        drawPolygon(points.begin(), points.size());
    }



    void drawHollowTriangle(const Vector2i & p0,const Vector2i & p1,const Vector2i & p2);

    void drawFilledTriangle(const Vector2i & p0,const Vector2i & p1,const Vector2i & p2);

    void drawRoi(const Rect2i & rect);
};

}