#ifndef __PAINTER_HPP__

#define __PAINTER_HPP__

#include "image.hpp"
#include "packed_image.hpp"

#include "types/rect2/rect2_t.hpp"
#include "types/color/color_t.hpp"
#include "types/rgb.h"

#include "font/font.hpp"
#include "types/string/String.hpp"

template<typename ColorType>
class Painter{
protected:
    ImageWritable<ColorType> * src_image = nullptr;
    const Font * chfont = &font7x7;
    const Font * enfont = &font8x5;
    ColorType m_color;

    void drawtexture_unsafe(const Rect2i & rect,const ColorType * color_ptr){
        src_image -> puttexture_unsafe(rect, color_ptr);
    }

public:
    Painter(){;}

    template<typename U>
    void setColor(U _color){
        m_color = _color;
    }
    
    void bindImage(ImageWritable<ColorType> & _source){
        src_image = &_source;
    }

    void setChFont(const Font & _chfont){
        chfont = &_chfont;
    }

    void setEnFont(const Font & _enfont){
        enfont = &_enfont;
    }


    void drawTextureRect(const Rect2i & rect,const ColorType * color_ptr){
        if(!src_image->getDisplayArea().contains(rect)) return;
        drawtexture_unsafe(rect, color_ptr);
    }



    void drawImage(ImageWithData<ColorType, ColorType> & image, const Vector2i & pos = Vector2i(0,0)){
        if(!src_image->get_window().contains(image.get_window()) || image.data == nullptr) return;
        drawtexture_unsafe(Rect2i(pos, image.get_size()), image.data.get());
    }

    template<typename w_ColorType, w_ColorType>
    void drawImage(ImageWithData<w_ColorType, w_ColorType> & image, const Vector2i & pos = Vector2i(0,0)){
        if(!src_image->get_window().contains(image.get_window()) || image.data == nullptr) return;
        auto rect = Rect2i(pos, image.get_size());
        src_image->setarea_unsafe(rect);
        uint32_t i = 0;
        w_ColorType * ptr = image.data.get();
        for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++, i++)
                putpixel_unsafe(Vector2i(x,y), ptr[i]);
        // drawtexture_unsafe(Rect2i(pos, image.get_size()), image.data.get());

    }

    void drawHriLine(const Rangei & x_range, const int & y){
        if(!x_range ||!src_image->get_window().get_y_range().has_value(y)) return;

        src_image -> putrect_unsafe(Rect2i(x_range, Rangei(y, y+1)), m_color);
    }

    void drawHriLine(const Vector2i & pos,const int &l){
        Rangei x_range = src_image->get_window().get_x_range().intersection(Rangei(pos.x, pos.x + ABS(l)));
        drawHriLine(x_range, pos.y);
    }

    void drawVerLine(const Vector2i & pos,const int &l){
        Rangei y_range = src_image->get_window().get_y_range().intersection(Rangei(pos.y, pos.y + ABS(l)));
        if(!y_range ||!src_image->get_window().get_x_range().has_value(pos.x)) return;

        src_image -> putrect_unsafe(Rect2i(Rangei(pos.x,pos.x+1), y_range), m_color);
    }

    void drawVerLine(const Rangei & y_range, const int & x){
        if(!y_range ||!src_image -> get_window().get_x_range().has_value(x)) return;
        src_image -> putrect_unsafe(Rect2i(Rangei(x,x+1), y_range), m_color);
    }

    void drawFilledRect(const Rect2i & rect, const ColorType & color){
        Rect2i rect_area = src_image->get_window().intersection(rect);
        if(!rect_area) return;
        src_image -> putrect_unsafe(rect_area, color);
    }

    void flush(){
        flush(m_color);
        // src_image -> putrect_unsafe(src_image->get_window(), m_color);
    }

    void flush(const ColorType & color){
        src_image -> putrect_unsafe(src_image->get_window(), color);
    }
    void drawPixel(const Vector2i & pos, const ColorType & color){
        src_image -> putpixel(pos, color);
    }

    void drawPixel(const Vector2i & pos){
        src_image -> putpixel(pos, m_color);
    }

    void drawLine(const Vector2i & start, const Vector2i & end){
        if(!src_image->has_point(start)){
            DEBUG_LOG("start point lost: ", start);
            return;
        }else if(!src_image->has_point(end)){
            DEBUG_LOG("end point lost: ", end);
            return;
        }
        auto [x0, y0] = start;
        auto [x1, y1] = end;
        bool steep = false;
        if (std::abs(x1 - x0) < std::abs(y1 - y0)) {
            std::swap(x0, y0);
            std::swap(x1, y1);
            steep = true;
        }
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        int dx = x1 - x0;
        int dy = y1 - y0;
        int deltaY = ABS(dy << 1);
        int middle = dx;
        int y = y0;
        for (int x = x0; x <= x1; ++x) {
            if (steep) {
                drawPixel({y,x});
            }
            else {
                drawPixel({x,y});
            }
            deltaY += std::abs(dy << 1);
            if (deltaY >= middle) {
                y += (y1 > y0 ? 1 : -1);
                middle += std::abs(dx << 1);
            }
        }
    }

    void drawHollowRect(const Rect2i & rect){
        Rect2i regular = rect.abs();
        if(!src_image -> get_window().intersects(regular)) return;

        Rangei x_range = regular.get_x_range();
        Rangei y_range = regular.get_y_range();

        if(y_range.length() > 2){
            drawHriLine(x_range, y_range.start);
            drawHriLine(x_range, y_range.end - 1);
            Rangei shrunk_y_range = y_range.grow(-1);
            drawVerLine(shrunk_y_range, x_range.start);
            drawVerLine(shrunk_y_range, x_range.end - 1);
        }else{
            drawFilledRect(Rect2i(x_range, y_range), m_color);
        }
    }

    void drawHollowCircle(const Vector2i & pos, const int & radius){
        if((!(Rect2i::from_center(pos, Vector2i(radius, radius)).intersects(src_image->area))) || radius <= 0) return;

        int x0 = pos.x;
        int y0 = pos.y;
        int x = radius - 1;
        int y = 0;
        int dx = 1;
        int dy = 1;
        int err=dx - 2 * radius;

        while (x>=y) {
            src_image -> putPixel(Vector2i(x0-x, y0+y), m_color);
            src_image -> putPixel(Vector2i(x0+x, y0+y), m_color);
            src_image -> putPixel(Vector2i(x0-y, y0+x), m_color);
            src_image -> putPixel(Vector2i(x0+y, y0+x), m_color);
            src_image -> putPixel(Vector2i(x0-x, y0-y), m_color);
            src_image -> putPixel(Vector2i(x0+x, y0-y), m_color);
            src_image -> putPixel(Vector2i(x0-y, y0-x), m_color);
            src_image -> putPixel(Vector2i(x0+y, y0-x), m_color);

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
        // if(src_image !)
        if((!(Rect2i::from_center(pos, Vector2i(radius, radius)).inside(Rect2i(Vector2i(), src_image->size)))) || radius == 0) return;
        if(radius == 1){
            return drawPixel(pos);
        }
        int x0 = pos.x;
        int y0 = pos.y;
        int x = ABS(radius) - 1;
        int y = 0;
        int dx = 1;
        int dy = 1;
        int err=dx - 2 * ABS(radius);

        while (x>=y) {
            // if(src_image->has_point(x0 - y))
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
                err += dx-ABS(radius * 2);
            }


        }
    }

    void drawChar(const Vector2i & pos,const wchar_t & chr){
        Rect2i image_area = Rect2i({}, src_image->size);
        const Font * font = chr > 0x80 ? chfont : enfont;
        const Vector2i font_size = font->size;
        Rect2i char_area = Rect2i(pos, font_size).intersection(image_area);

        if(!char_area) return;
        
        for(int i = char_area.position.x; i < char_area.position.x + char_area.size.x ; i++){
            uint8_t mask;
            for(int j = 0; j < font_size.y; j++){
 
                if(j % 8 == 0) mask = 0;

                Vector2i offs = Vector2i(i - char_area.position.x ,j % 8);
                if(font->getpixel(chr, offs)){
                    mask |= (0x01 << (j % 8));
                }

                if(j % 8 == 7 || j == font_size.y - 1){
                    src_image->putseg_v8_unsafe(Vector2i(i, (j & (~(8 - 1))) + pos.y), mask, m_color);
                }
            }
        }
    }

    void drawString(const Vector2i & pos, const String & str){
    const char * str_ptr = str.c_str();
	GBKIterator iterator(str_ptr);
	
    for(int x = pos.x; x < src_image->size.x;){
        if(iterator.hasNext()){
            auto chr = iterator.next();
            drawChar(Vector2i(x, pos.y), chr);
            x += ((chr > 0x80 ? chfont->size.x : enfont->size.x) + 1);
        }else{
            break;
        }
    }
}

    void drawHollowEllipse(const Vector2i & pos, const Vector2i & r) {
        int rx = r.x;
        int ry = r.y;
        if (rx == ry) return drawHollowCircle(pos, rx);
        if (rx<2 || ry<2|| !src_image->area.intersects(Rect2i::from_center(pos, r))) return;

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
        if (rx<2 || ry<2|| !src_image->area.intersects(Rect2i::from_center(pos, r))) return;

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