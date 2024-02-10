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
    virtual void putPixel_Unsafe(const Vector2i & pos, const ColorType & color) = 0;
    virtual void putPixel(const Vector2i & pos, const ColorType & color) = 0;
    virtual void putPixelCont(const ColorType & color) = 0;
    virtual void putTexture_Unsafe(const Rect2i & area, const ColorType * color_ptr) = 0;
    virtual void putRect_Unsafe(const Rect2i & area, const ColorType & color) = 0;
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

        putPixel(shade_area.position, callback(Vector2i(0,0)));
        for(int x = 1; x < shade_area.size.y; x++){
            putPixelCont(callback(Vector2i(x, 0)));
        }
        for(int y = 1; y < shade_area.size.x; y++){
            for(int x = 0; x < area.size.y; x++){
                putPixelCont(callback(Vector2i(x, y)));
            }
        }
    }

    void shade(UVShaderCallback callback, const Rect2i & _shade_area ){
        Rect2i shade_area = _shade_area.intersection(area);

        putPixel(shade_area.position, callback(Vector2(0,0)));
        // real_t dx = real_t(1)/shade_area.size.x;
        // real_t dy = real_t(1)/shade_area.size.y;

        // real_t x = dx;
        // for(int i = 1; i < shade_area.size.x; i++){
        //     putPixelCont(callback(Vector2(x, real_t(0))));
        //     x += dx;
        // }

        // real_t y = dy;
        // x = real_t(0);
        SPI_DataSizeConfig(SPI2, SPI_DataSize_16b);
        int j_m = shade_area.size.y;
        int x_m = shade_area.size.x;
        static t = -
        for(int j = 1; j < j_m; j++){
            for(int i = 0; i < x_m; i++){
                // putPixelCont(callback(Vector2(x, y)));
                // putPixelCont((RGB565)(i+j));
                
                while ((SPI2->STATR & SPI_I2S_FLAG_TXE) == RESET);
                SPI2->DATAR = i+j;
                // x += dx;
            }
            // y += dy;
        }
    }
};

class Image565 : public Image<RGB565>{

};
#endif