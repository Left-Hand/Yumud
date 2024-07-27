#pragma once


#include "../types/vector2/vector2_t.hpp"
#include "../types/rect2/rect2_t.hpp"
#include "../types/color/color_t.hpp"
#include "../types/rgb.h"
#include <functional>
#include <memory>


template <typename ColorType>
class Painter;

template <typename ColorType>
class ImageWR;


template <typename ColorType>
class PixelProxy;

class ImageBasics{
public:
    union{
        Vector2i size;
        struct{
            int w; 
            int h;
        };
        struct{
            int width;
            int height;
        };
    };

    ImageBasics(const Vector2i & _size):size(_size){;}

    Vector2i uv2pixel(const Vector2 & uv) const{
        return Vector2i(int(LERP((uv.x + 1) / 2, 0, this->size.x)), int(LERP((uv.y + 1) / 2, 0, this->size.y)));
    }

    Vector2 uv2aero(const Vector2 & uv) const{
        return Vector2(((uv.x + 1) * (this->size.x / 2)), (uv.y + 1) * (this->size.y / 2));
    }

    Vector2 pixel2uv(const Vector2i & pixel)const{
        return Vector2(INVLERP((real_t)pixel.x, this->size.x / 2, this->size.x), INVLERP((real_t)pixel.y, this->size.y / 2, this->size.y));
    }

    Vector2 uvstep() const{
        return Vector2(real_t(2) / this->size.x, real_t(2) / this->size.y);
    }

    bool has_point(const Vector2i & pos) const{
        return size.has_point(pos);
    }

    virtual Rect2i get_view() const{
        return Rect2i(Vector2i(), size);
    }

    __fast_inline Vector2i get_size() const{
        return this->size;
    }


};

template<typename ColorType>
class ImageReadable:virtual public ImageBasics{
protected:
    virtual void getpixel_unsafe(const Vector2i & pos, ColorType & color) const = 0;
    void getpixel(const Vector2i & pos, ColorType & color) const{
        if(this->has_point(pos)){
            getpixel_unsafe(pos, color);
        }
    }

    uint8_t getseg_v8() const{return 0;}
    uint8_t getseg_h8() const{return 0;}
public:
    ImageReadable(const Vector2i & _size):ImageBasics(_size){;}

    __fast_inline ColorType operator[](const Vector2i & pos)const{
        ColorType color;
        getpixel(pos, color);
        return color;
    }

    __fast_inline ColorType operator[](const size_t & index)const{
        ColorType color;
        getpixel(Vector2i(index % ImageBasics::get_size().x, index / ImageBasics::get_size().x), color);
        return color;
    }

    __fast_inline ColorType at(const int y, const int x)const{
        ColorType color;
        getpixel({x,y}, color);
        return color;
    }

};

template<typename ColorType>
class ImageWritable:virtual public ImageBasics{
protected:
    friend class Painter<ColorType>;

    virtual void setpos_unsafe(const Vector2i & pos) = 0;
    virtual void setarea_unsafe(const Rect2i & rect) = 0;
    virtual void putpixel_unsafe(const Vector2i & pos, const ColorType & color) = 0;

    virtual void puttexture_unsafe(const Rect2i & rect, const ColorType * color_ptr){
        setarea_unsafe(rect);
        uint32_t i = 0;
        for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++, i++)
                putpixel_unsafe(Vector2i(x,y), color_ptr[i]);
    }

    virtual void putrect_unsafe(const Rect2i & rect, const ColorType & color){
        
        setarea_unsafe(rect);
        for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++)
                putpixel_unsafe(Vector2i(x,y), color);
    }

    void putpixel(const Vector2i & pos, const ColorType & color){
        if(this->has_point(pos)){
            putpixel_unsafe(pos, color);
        }
    }

    virtual void putseg_v8_unsafe(const Vector2i & pos, const uint8_t & mask, const ColorType & color){
        Rect2i area(pos, Vector2i(1, 8));
        if(Rect2i(this->size, Vector2i()).contains(area)){
            for(int i = 0; i < 8; i++){
                if(mask & (1 << i)) putpixel_unsafe(pos + Vector2i{0,i}, color);
            }
        }else{
            for(int i = 0; i < 8; i++){
                if(mask & (1 << i))putpixel(pos + Vector2i{0,i}, color);
            }
        }
    }

    virtual void putseg_h8_unsafe(const Vector2i & pos, const uint8_t & mask, const ColorType & color){
        Rect2i area(pos, Vector2i(8, 1));
        if(Rect2i(this->size, Vector2i()).contains(area)){
            for(int i = 0; i < 8; i++){
                if(mask & (0x80 >> i))putpixel_unsafe(pos + Vector2i{i,0}, color);
            }
        }else{
            for(int i = 0; i < 8; i++){
                if(mask & (1 << i))putpixel(pos + Vector2i{i,0}, color);
            }
        }
    }

    friend class PixelProxy<ColorType>;
public:
    ImageWritable(const Vector2i & _size):ImageBasics(_size){;}

    void setpos(const Vector2i & pos){if(this->has_point(pos)){
        setpos_unsafe(pos);
    }}

    void fill(const ColorType color){
        putrect_unsafe(Rect2i{Vector2i{}, ImageBasics::get_size()}, color);
    }

    void putrect(const Rect2i & rect, const ColorType color){
        auto area = rect.intersection(this->get_view());
        putrect_unsafe(area, color);
    }

    void puttexture(const Rect2i & rect, const ColorType * color_ptr){
        if(rect.inside(this->get_view())){
            puttexture_unsafe(rect, color_ptr);
        }else{
            auto area = rect.intersection(this->get_view());
            if(bool(area) == false) return;
            setarea_unsafe(area);
            uint32_t i = 0;
            for(int x = area.position.x; x < area.position.x + area.size.x; x++)
                for(int y = area.position.y; y < area.position.y + area.size.y; y++, i++)
                    putpixel_unsafe(Vector2i(x,y), color_ptr[i]);
        }
    }


    __fast_inline PixelProxy<ColorType> operator[](const Vector2i & pos){
        return PixelProxy<ColorType>(*this,pos);
    }

    __fast_inline PixelProxy<ColorType> operator[](const size_t index){
        return PixelProxy<ColorType>(*this,Vector2i(index % ImageBasics::get_size().x, index / ImageBasics::get_size().x));
    }
};


template<typename ColorType>
class ImageWR:public ImageReadable<ColorType>, public ImageWritable<ColorType>{
protected:
    using PixelShaderCallback = ColorType(*)(const Vector2i &);
    using UVShaderCallback = ColorType(*)(const Vector2 &);

    friend class Painter<ColorType>;
public:
    ImageWR(const Vector2i & _size):ImageReadable<ColorType>(_size), ImageWritable<ColorType>(_size){;}
};


template<typename ColorType, typename DataType>
class ImageWithData : public ImageWR<ColorType> {
protected:
    Rect2i select_area;

    void setpos_unsafe(const Vector2i & pos) override { select_area.position = pos; }
    void setarea_unsafe(const Rect2i & rect) override { select_area = rect; }
    void putpixel_unsafe(const Vector2i & pos, const ColorType & color) override { data[this->size.x * pos.y + pos.x] = color; }
    void getpixel_unsafe(const Vector2i & pos, ColorType & color) const override { color = data[this->size.x * pos.y + pos.x]; }

    std::shared_ptr<DataType[]> data;


public:
    ImageWithData(std::shared_ptr<DataType[]> _data, const Vector2i & _size) : ImageBasics(_size), ImageWR<ColorType>(_size), data(_data) {;}
    ImageWithData(const Vector2i & _size) : ImageBasics(_size), ImageWR<ColorType>(_size), data(std::make_shared<DataType[]>(_size.x * _size.y)) {;}

    // Move constructor
    ImageWithData(ImageWithData&& other) noexcept : ImageBasics(other.size), ImageWR<ColorType>(other.size), data(std::move(other.data)){}


    ImageWithData(const ImageWithData& other) noexcept : ImageBasics(other.size), ImageWR<ColorType>(other.size), data(other.data){}


    // Move assignment operator
    ImageWithData& operator=(ImageWithData&& other) noexcept {
        if (this != &other) {
            this->size = std::move(other.size);
            this->select_area = std::move(other.select_area);
            this->data = std::move(other.data);
        }
        return *this;
    }


    __fast_inline const DataType& operator[](const size_t & index) const { return data[index]; }
    __fast_inline const ColorType& operator[](const Vector2i & pos) const { return data[pos.x + pos.y * ImageBasics::w]; }

    __fast_inline DataType& operator[](const size_t & index) { return data[index]; }
    __fast_inline ColorType& operator[](const Vector2i & pos) { return data[pos.x + pos.y * ImageBasics::w]; }


    template<typename ToColorType>
    __fast_inline ToColorType at(const int y, const int x) const { return data[x + y * ImageBasics::w]; }

    __fast_inline ColorType & at(const int y, const int x){ return data[x + y * ImageBasics::w]; }


    bool operator == (const ImageWithData<auto, auto> & other) const {
        return data == other.data;
    }
};

