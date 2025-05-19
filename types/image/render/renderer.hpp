#pragma once

#include "core/stream/stream.hpp"

#include "types/image/image.hpp"
#include "types/rect2/rect2.hpp"
#include "types/color/color_t.hpp"
#include "types/rgb.h"


namespace ymd{


class Renderer{
protected:
    using ColorType = RGB565;
    using Canvas = ImageWritable<RGB565>; 


    Canvas * src_ = nullptr;
    Rect2u clip_ = Rect2u{0, 0, 0xffff, 0xffff};

    ColorType color_ = ColorEnum::WHITE;
    // ColorType stroke_color_ = ColorEnum::WHITE;
public:
    Renderer(){;}
    void bind(Canvas & _source);
    void set_clip(const Rect2u clip){clip_ = clip;}
    void set_color(const ColorType color){color_ = color;}

    void draw_rect(const Rect2u rect);
    void draw_pixel(const Vector2u pos);
    void draw_line(const Vector2u from, const Vector2u to);
    void draw_hri_line(const Vector2u pos,const int l);
    void draw_ver_line(const Vector2u pos,const int l);
    void draw_ver_line(const Range2i y_range, const int x);
    void draw_hri_line(const Range2i x_range, const int y);
};

}