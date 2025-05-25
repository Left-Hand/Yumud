#pragma once

#include <functional>
#include <memory>

#include "core/utils/Result.hpp"

#include "types/vectors/vector2/vector2.hpp"
#include "types/regions/rect2/rect2.hpp"
#include "types/colors/color/color.hpp"
#include "types/colors/rgb/rgb.hpp"




namespace ymd{

    // class Painter;
    
template <typename ColorType>
class Painter;

template <typename ColorType>
class ImageWR;


template <typename ColorType>
class PixelProxy;



// class ImageBasics{
// private:
//     Vector2u size_;
// protected:
//     __fast_inline void set_size(const Vector2u size){
//         size_ = size;
//     }
// public:

//     ImageBasics(const Vector2u & size):size_(size){;}



//     __fast_inline Vector2u size() const{
//         return this->size_;
//     }


// };

template<typename ColorType>
class ImageReadableIntf{
protected:
    virtual void getpixel_unsafe(const Vector2u & pos, ColorType & color) const = 0;
    __fast_inline void getpixel(const Vector2u & pos, ColorType & color) const{
        getpixel_unsafe(pos, color);
    }
public:
    virtual ~ImageReadableIntf() = 0;

    __fast_inline ColorType operator[](const Vector2u & pos)const{
        ColorType color;
        getpixel(pos, color);
        return color;
    }

    __fast_inline ColorType at(const size_t y, const size_t x)const{
        ColorType color;
        getpixel({x,y}, color);
        return color;
    }

};



template<typename ColorType>
class ImageWritableIntf{
public:
    virtual ~ImageWritableIntf() = 0;

    void fill(const ColorType color){
        putrect_unsafe(Rect2u{Vector2u{0,0}, this->size()}, color);
    }

    void put_rect(const Rect2u & rect, const ColorType color){
        auto area = rect.intersection(this->rect());
        putrect_unsafe(area, color);
    }

    virtual void put_texture(const Rect2u & rect, const ColorType * color_ptr){
        if(rect.is_inside(this->size().to_rect())){
            puttexture_unsafe(rect, color_ptr);
        }else{
            const auto ins_opt = rect.intersection(this->size().to_rect());
            if(ins_opt.is_none()) return;
            const auto ins = ins_opt.unwrap();
            setarea_unsafe(ins);

            size_t i = 0;
            for(size_t x = ins.position.x; x < ins.position.x + ins.size.x; x++){
                for(size_t y = ins.position.y; y < ins.position.y + ins.size.y; y++){
                    putpixel_unsafe(Vector2u(x,y), color_ptr[i]);
                    i++;
                }
            }
        }
    }

    __fast_inline PixelProxy<ColorType> operator[](const Vector2u & pos){
        return PixelProxy<ColorType>(*this,pos);
    }
protected:

    virtual void putpixel_unsafe(const Vector2u & pos, const ColorType color) = 0;

    virtual void puttexture_unsafe(const Rect2u & rect, const ColorType * color_ptr){
        uint32_t i = 0;
        for(size_t x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(size_t y = rect.position.y; y < rect.position.y + rect.size.y; y++, i++)
                putpixel_unsafe(Vector2u(x,y), color_ptr[i]);
    }

    virtual void putrect_unsafe(const Rect2u & rect, const ColorType color){
        for(size_t x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(size_t y = rect.position.y; y < rect.position.y + rect.size.y; y++)
                putpixel_unsafe(Vector2u(x,y), color);
    }
    void putpixel(const Vector2u & pos, const ColorType & color){
        if(this->size().has_point(pos)){
            putpixel_unsafe(pos, color);
        }
    }

    virtual void putseg_v8_unsafe(
        const Vector2u & pos, 
        const uint8_t mask, 
        const ColorType color
    ){
        if(true){
            for(size_t i = 0; i < 8; i++){
                if(mask & (1 << i)) putpixel_unsafe(pos + Vector2u{0u,i}, color);
            }
        }else{
            for(size_t i = 0; i < 8; i++){
                if(mask & (1 << i))putpixel(pos + Vector2u{0u,i}, color);
            }
        }
    }

    virtual void putseg_h8_unsafe(
        const Vector2u & pos, 
        const uint8_t mask, 
        const ColorType color
    ){
        if(true){
            for(size_t i = 0; i < 8; i++){
                if(mask & (0x80 >> i))putpixel_unsafe(pos + Vector2u{i,0}, color);
            }
        }else{
            for(size_t i = 0; i < 8; i++){
                if(mask & (1 << i))putpixel(pos + Vector2u{i,0}, color);
            }
        }
    }

    friend class Painter<ColorType>;
    friend class PixelProxy<ColorType>;
};


}