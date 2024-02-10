#ifndef __IMAGE_HPP__

#define __IMAGE_HPP__

#include "../rect2/rect2_t.hpp"
#include "../color/color_t.hpp"
#include "../rgb.h"
#include <functional>

template <typename ColorType>
class Painter;

template<typename ColorType>
class Image{
protected:
    Rect2i area = Rect2i();
    Image(){;}

    virtual void setPosition_Unsafe(const Vector2i & pos) = 0;
    virtual void setArea_Unsafe(const Rect2i & rect) = 0;
    virtual void putPixel_Unsafe(const Vector2i & pos, const ColorType & color) = 0;
    virtual void putPixel(const Vector2i & pos, const ColorType & color) = 0;

    virtual void putPixels(const RGB565 & data, const size_t & len) = 0;
    virtual void putPixels(const RGB565 * data_ptr, const size_t & len) = 0;
    void putTexture_Unsafe(const Rect2i & rect, const RGB565 * color_ptr){
        setArea_Unsafe(rect);
        putPixels(color_ptr, rect.get_area());
    }

    void putRect_Unsafe(const Rect2i & rect, const RGB565 & color){
        setArea_Unsafe(rect);
        putPixels(color, rect.get_area());
    }
    virtual ColorType takePixel(const Vector2i & pos) const = 0;

    using PixelShaderCallback = ColorType(*)(const Vector2i &);
    using UVShaderCallback = ColorType(*)(const Vector2 &);

    friend class Painter<ColorType>;
public:
    Rect2i get_display_area() const {
        return area;
    }

    void shade(PixelShaderCallback callback, const Rect2i & _shade_area){
        Rect2i shade_area = _shade_area.intersection(area);
        if(!area) return;
        size_t buflen = shade_area.size.x;
        ColorType * buf = new ColorType[buflen];

        setArea_Unsafe(shade_area);
        for(int y = 0; y < shade_area.size.y; y++){
            for(int x = 0; x < shade_area.size.x; x++){
                buf[x] = (callback(Vector2i(x, y)));
            }
            putPixels(buf, buflen);
        }

        delete[] buf;
    }

    void shade(UVShaderCallback callback, const Rect2i & _shade_area){
        Rect2i shade_area = _shade_area.intersection(area);
        if(!area) return;
        size_t buflen = shade_area.size.x;
        ColorType * buf = new ColorType[buflen];

        setArea_Unsafe(shade_area);
        real_t fx, fy;
        real_t dx = real_t(1) / shade_area.size.x;
        real_t dy = real_t(1) / shade_area.size.y;

        for(int y = 0; y < shade_area.size.y; y++){
            fx = real_t(0);
            for(int x = 0; x < shade_area.size.x; x++){
                buf[x] = (callback(Vector2(fx, fy)));
                fx += dx;
            }
            putPixels(buf, buflen);
            fy += dy;
        }

        delete[] buf;
    }
};

class Image565 : public Image<RGB565>{

};
#endif