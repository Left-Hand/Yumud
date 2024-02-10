#ifndef __IMAGE_HPP__

#define __IMAGE_HPP__

#include "../rect2/rect2_t.hpp"
#include "../color/color_t.hpp"
#include "../rgb.h"

template <typename ColorType>
class Painter;

template<typename ColorType>
class Image{
protected:
    Rect2i area = Rect2i();
    Image(){;}
    virtual void putPixel_Unsafe(const Vector2i & pos, const ColorType & color) = 0;
    virtual void putPixel(const Vector2i & pos, const ColorType & color) = 0;
    virtual void putTexture_Unsafe(const Rect2i & area, const ColorType * color_ptr) = 0;
    virtual void putRect_Unsafe(const Rect2i & area, const ColorType & color) = 0;

    friend class Painter<ColorType>;
};

class Image565 : public Image<RGB565>{

};
#endif