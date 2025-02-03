#pragma once

#include "sys/stream/stream.hpp"

#include "types/image/image.hpp"
#include "types/rect2/rect2_t.hpp"
#include "types/color/color_t.hpp"
#include "types/rgb.h"


namespace ymd{


class Renderer{
protected:
    using ColorType = RGB565;
    using Vector2 = Image<ColorType>::Vector2;
    using Vector2i = Image<ColorType>::Vector2i;
    using Canvas = ImageWritable<RGB565>; 


    Canvas * src_ = nullptr;
    Rect2i clip_ = Rect2i{0, 0, 0xffff, 0xffff};

    ColorType color_ = ColorEnum::WHITE;
    // ColorType stroke_color_ = ColorEnum::WHITE;
public:
    Renderer(){;}
    void bind(Canvas & _source);
    void setClip(const Rect2i clip){clip_ = clip;}
    void setColor(const ColorType color){color_ = color;}

    void drawRect(const Rect2i rect);
    void drawPixel(const Vector2i pos);
    void drawLine(const Vector2i from, const Vector2i to);
    void drawHriLine(const Vector2i pos,const int l);
    void drawVerLine(const Vector2i pos,const int l);
    void drawVerLine(const Rangei y_range, const int x);
    void drawHriLine(const Rangei x_range, const int y);
};

}