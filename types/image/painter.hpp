#ifndef __PAINTER_HPP__

#define __PAINTER_HPP__

#include "image.hpp"
#include "font/font.hpp"
#include "../string/String.hpp"

template<typename ColorType>
class Painter{
protected:
    Image<ColorType> * srcImage = nullptr;
    Font * font = nullptr;
    ColorType color;
public:
    Painter(Image<ColorType> * _srcImage, Font * _font):srcImage(_srcImage),font(_font) {;}

    template<typename U>
    void setColor(U _color){
        color = _color;
    }

    void setFont(Font * _font){
        font = _font;
    }

    void setSource(Image<ColorType> & _source){
        srcImage = &_source;
    }

    void drawHriLine(const Vector2i & pos,const int &l){
        Rangei x_range = srcImage -> area.get_x_range().intersection(Rangei(pos.x, pos.x + l).abs());
        if(!x_range || !srcImage -> area.get_y_range().has_value(pos.y)) return;

        srcImage -> putRect_Unsafe(Rect2i(x_range, Rangei(pos.y, pos.y+1)), color);
    }

    void drawHriLine(const Rangei & x_range, const int & y){
        if(!x_range ||!srcImage -> area.get_y_range().has_value(y)) return;

        srcImage -> putRect_Unsafe(Rect2i(x_range, Rangei(y, y+1)), color);
    }

    void drawVerLine(const Vector2i & pos,const int &l){
        Rangei y_range = srcImage -> area.get_y_range().intersection(Rangei(pos.y, pos.y + l).abs());
        if(!y_range ||!srcImage -> area.get_x_range().has_value(pos.x)) return;

        srcImage -> putRect_Unsafe(Rect2i(Rangei(pos.x,pos.x+1), y_range), color);
    }

    void drawVerLine(const Rangei & y_range, const int & x){
        if(!y_range ||!srcImage -> area.get_x_range().has_value(x)) return;
        srcImage -> putRect_Unsafe(Rect2i(Rangei(x,x+1), y_range), color);
    }

    void drawFilledRect(const Rect2i & rect){
        Rect2i rect_area = srcImage -> area.intersection(rect);
        if(!rect_area) return;
        srcImage -> putRect_Unsafe(rect_area, color);
    }

    void flush(){
        srcImage -> putRect_Unsafe(srcImage->area, color);
    }
    void drawPixel(const Vector2i & pos){
        srcImage -> putPixel(pos, color);
    }

    void drawLine(const Vector2i & start, const Vector2i & end){
        int x0 = start.x;
        int y0 = start.y;
        int x1 = end.x;
        int y1 = end.y;

        if(y0 == y1){
            drawHriLine(start, x1 - x0);
            return;
        }else if (x0 == x1){
            drawVerLine(start, y1 - y0);
            return;
        }

        int16_t dx=ABS(x1-x0);
        int8_t sx=(x1 > x0)? 1: -1;
        int16_t dy=ABS(y1-y0);
        int8_t sy=(y1 > y0)? 1 : -1;

        int16_t err=(dx>dy ? dx : -dy)/2;
        int16_t e2;

        int16_t x = x0;
        int16_t y = y0;

        while(1){
            srcImage -> putPixel(Vector2i(x,y), color);
            if (x==x1 && y==y1)
                break;

            e2=err;
            if (e2>-dx) {
                err-=dy;
                x+=sx;
            }
            if (e2<dy) {
                err+=dx;
                y+=sy;
            }
        }
    }

    void drawHollowRect(const Rect2i & rect){
        Rect2i regular = rect.abs();
        if(!srcImage -> area.intersects(regular)) return;

        Rangei x_range = regular.get_x_range();
        Rangei y_range = regular.get_y_range();

        if(y_range.get_length() > 2){
            drawHriLine(x_range, y_range.start);
            drawHriLine(x_range, y_range.end - 1);
            Rangei shrunk_y_range = y_range.grow(-1);
            drawVerLine(shrunk_y_range, x_range.start);
            drawVerLine(shrunk_y_range, x_range.end - 1);
        }else{
            drawFilledRect(Rect2i(x_range, y_range));
        }
    }

    void drawHollowCircle(const Vector2i & pos, const int & radius){
        if((!(Rect2i::from_center(pos, Vector2i(radius, radius)).intersects(srcImage->area))) || radius <= 0) return;

        int x0 = pos.x;
        int y0 = pos.y;
        int x = radius - 1;
        int y = 0;
        int dx = 1;
        int dy = 1;
        int err=dx - 2 * radius;

        while (x>=y) {
            srcImage -> putPixel(Vector2i(x0-x, y0+y), color);
            srcImage -> putPixel(Vector2i(x0+x, y0+y), color);
            srcImage -> putPixel(Vector2i(x0-y, y0+x), color);
            srcImage -> putPixel(Vector2i(x0+y, y0+x), color);
            srcImage -> putPixel(Vector2i(x0-x, y0-y), color);
            srcImage -> putPixel(Vector2i(x0+x, y0-y), color);
            srcImage -> putPixel(Vector2i(x0-y, y0-x), color);
            srcImage -> putPixel(Vector2i(x0+y, y0-x), color);

            if (err<=0) {
                y++;
                err += dy;
                dy += 2;
            }
            if (err>0) {
                x--;
                dx += 2;
                err += dx-radius * 2;
            }
        }
    }

    void drawFilledCircle(const Vector2i & pos, const int & radius){
        if((!(Rect2i::from_center(pos, Vector2i(radius, radius)).intersects(srcImage->area))) || radius <= 0) return;

        int x0 = pos.x;
        int y0 = pos.y;
        int x = radius - 1;
        int y = 0;
        int dx = 1;
        int dy = 1;
        int err=dx - 2 * radius;

        while (x>=y) {
            drawHriLine(Vector2i(x0 - x, y0 + y), 2*x);
            drawHriLine(Vector2i(x0 - y, y0 + x), 2*y);
            drawHriLine(Vector2i(x0 - x, y0 - y), 2*x);
            drawHriLine(Vector2i(x0 - y, y0 - x), 2*y);

            if (err<=0) {
                y++;
                err += dy;
                dy += 2;
            }
            if (err>0) {
                x--;
                dx += 2;
                err += dx-radius * 2;
            }
        }
    }

    void drawChar(const Vector2i & pos,char chr){
        Vector2i size = font->size;
        Rect2i char_area = Rect2i(pos, size).intersection(srcImage->area);
        if(!char_area) return;

        for(uint8_t i = 0; i < char_area.size.x ; i++){
            for(uint8_t j = 0; j < char_area.size.y; j++){
                Vector2i offs = Vector2i(i,j);
                if(font->getPixel(chr, offs)){
                    srcImage->putPixel_Unsafe(char_area.position + offs, color);
                }
            }
        }
    }

    void drawString(const Vector2i & pos, const String & str){
    const char * str_ptr = str.c_str();

    for(int x = pos.x; x < srcImage -> area.size.x; x += (font->size.x + 1)){
        if(*str_ptr){
            drawChar(Vector2i(x, pos.y), *str_ptr);
        }else{
            break;
        }
        str_ptr++;
    }
}

    void drawHollowEllipse(const Vector2i & pos, const Vector2i & r) {
        int rx = r.x;
        int ry = r.y;
        if (rx == ry) return drawHollowCircle(pos, rx);
        if (rx<2 || ry<2|| !srcImage->area.intersects(Rect2i::from_center(pos, r))) return;

        int x0 = pos.x;
        int y0 = pos.y;
        int x, y;
        int rx2 = rx * rx;
        int ry2 = ry * ry;
        int fx2 = 4 * rx2;
        int fy2 = 4 * ry2;
        int s;

        for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
            drawPixel(Vector2i(x0 + x, y0 + y));
            drawPixel(Vector2i(x0 - x, y0 + y));
            drawPixel(Vector2i(x0 - x, y0 - y));
            drawPixel(Vector2i(x0 + x, y0 - y));
            if (s >= 0) {
                s += fx2 * (1 - y);
                y--;
            }
            s += ry2 * ((4 * x) + 6);
        }

        for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
            drawPixel(Vector2i(x0 + x, y0 + y));
            drawPixel(Vector2i(x0 - x, y0 + y));
            drawPixel(Vector2i(x0 - x, y0 - y));
            drawPixel(Vector2i(x0 + x, y0 - y));
            if (s >= 0)
            {
                s += fy2 * (1 - x);
                x--;
            }
            s += rx2 * ((4 * y) + 6);
        }
    }

    void drawFilledEllipse(const Vector2i & pos, const Vector2i & r){
        int rx = r.x;
        int ry = r.y;
        if (rx == ry) return drawHollowCircle(pos, rx);
        if (rx<2 || ry<2|| !srcImage->area.intersects(Rect2i::from_center(pos, r))) return;

        int x0 = pos.x;
        int y0 = pos.y;

        int x, y;
        int rx2 = rx * rx;
        int ry2 = ry * ry;
        int fx2 = 4 * rx2;
        int fy2 = 4 * ry2;
        int s;

        for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
            Rangei x_range = Rangei(x0 - x, x0 + x + 1);
            drawHriLine(x_range, y0 - y);
            drawHriLine(x_range, y0 + y);

            if (s >= 0) {
                s += fx2 * (1 - y);
                y--;
            }
            s += ry2 * ((4 * x) + 6);
        }

        for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
            Rangei x_range = Rangei(x0 - x, x0 + x + 1);
            drawHriLine(x_range, y0 - y);
            drawHriLine(x_range, y0 + y);

            if (s >= 0) {
                s += fy2 * (1 - x);
                x--;
            }
            s += rx2 * ((4 * y) + 6);
        }
    }

    void drawPolyLine(const std::initializer_list<Vector2i> &points){
        if(points.size() < 2) return drawLine(*points.begin(), *points.end());

        auto prev = points.begin();
        for(auto it = points.begin() + 1; it!= points.end(); it++){
            drawLine(*prev, *it);
            prev = it;
        }
        drawLine(*prev, *points.begin());
    }

    void drawPolyLine(const std::vector<Vector2i> &points){
        if(points.size() < 2) return drawLine(*points.begin(), *points.end());
    }

    void drawHollowTriangle(Vector2i p0, Vector2i p1, Vector2i p2){
        drawLine(p0, p1);
        drawLine(p1, p2);
        drawLine(p0, p2);
    }

    void drawFilledTriangle(Vector2i p0, Vector2i p1, Vector2i p2){
        int a, b, y, last;
        int x0 = p0.x;
        int y0 = p0.y;
        int x1 = p1.x;
        int y1 = p1.y;
        int x2 = p2.x;
        int y2 = p2.y;

        if (y0 > y1) {
            SWAP(y0, y1);
            SWAP(x0, x1);
        }
        if (y1 > y2) {
            SWAP(y2, y1);
            SWAP(x2, x1);
        }
        if (y0 > y1) {
            SWAP(y0, y1);
            SWAP(x0, x1);
        }

        if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
            a = b = x0;
            if (x1 < a)      a = x1;
            else if (x1 > b) b = x1;
            if (x2 < a)      a = x2;
            else if (x2 > b) b = x2;
            drawHriLine(Rangei(a, b + 1), y);
            return;
        }


        int
        dx01 = x1 - x0,
        dy01 = y1 - y0,
        dx02 = x2 - x0,
        dy02 = y2 - y0,
        dx12 = x2 - x1,
        dy12 = y2 - y1,
        sa   = 0,
        sb   = 0;

        if (y1 == y2) last = y1;  // Include y1 scanline
        else         last = y1 - 1; // Skip it

        for (y = y0; y <= last; y++) {
            a   = x0 + sa / dy01;
            b   = x0 + sb / dy02;
            sa += dx01;
            sb += dx02;

            if (a > b) SWAP(a, b);
            drawHriLine(Rangei(a, b + 1), y);
        }

        sa = dx12 * (y - y1);
        sb = dx02 * (y - y0);
        for (; y <= y2; y++) {
            a   = x1 + sa / dy12;
            b   = x0 + sb / dy02;
            sa += dx12;
            sb += dx02;

            if (a > b) SWAP(a, b);
            drawHriLine(Rangei(a, b + 1), y);
        }
    }
};
#endif