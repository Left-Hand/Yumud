#include "Renderer.hpp"

using namespace ymd;

#define PUT_RECT(rect, color)     src_ -> putrect_unsafe(rect, color);
#define PUT_PIXEL(pos, color)       src_ -> putpixel_unsafe(pos, color);

void Renderer::bind(Canvas & _source){
    src_ = &_source;
    set_clip(clip_.intersection(_source.rect()));
}

void Renderer::draw_rect(const Rect2i rect) {
    Rect2i ins = clip_.intersection(rect);
    if(ins.get_area() == 0) return;

    PUT_RECT(ins, color_);
}

void Renderer::draw_pixel(const Vector2i pos) {
    if(!clip_.has_point(pos));

    PUT_PIXEL(pos, color_);
}

void Renderer::draw_line(const Vector2i from, const Vector2i to) {
    if(!src_->size().has_point(from)){
        return;
    }else if(!src_->size().has_point(to)){
        return;
    }

    auto [x0, y0] = from;
    auto [x1, y1] = to;

    if(y0 == y1) return draw_hri_line(from, x1 - x0);
    if(x0 == x1) return draw_ver_line(from, y1 - y0);
    bool steep = false;

    if (ABS(x1 - x0) < ABS(y1 - y0)) {
        SWAP(x0, y0);
        SWAP(x1, y1);
        steep = true;
    }

    if (x0 > x1) {
        SWAP(x0, x1);
        SWAP(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int deltaY = ABS(dy << 1);
    int middle = dx;
    int y = y0;
    for (int x = x0; x <= x1; ++x) {
        if (steep) {
            draw_pixel({y,x});
        }
        else {
            draw_pixel({x,y});
        }
        deltaY += ABS(dy << 1);
        if (deltaY >= middle) {
            y += (y1 > y0 ? 1 : -1);
            middle += ABS(dx << 1);
        }
    }
}

//绘制一条水平线
void Renderer::draw_hri_line(const Vector2i pos,const int l){
    auto rect = Rect2i(pos, Vector2i(l, 1));
    if(rect.get_area() == 0) return;
    draw_rect(rect);
}

//绘制一条水平线
void Renderer::draw_hri_line(const Range2i x_range, const int y){
    draw_hri_line(Vector2i(x_range.from, y), x_range.length());
}

//绘制一条垂直线
void Renderer::draw_ver_line(const Vector2i pos,const int l){
    auto rect = Rect2i(pos, Vector2i(1, l));
    if(rect.get_area() == 0) return;
    PUT_RECT(pos, color_)
}

//绘制一条垂直线
void Renderer::draw_ver_line(const Range2i y_range, const int x){
    draw_ver_line(Vector2i(x, y_range.from), y_range.length());
}

