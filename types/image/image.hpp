#ifndef __IMAGE_HPP__

#define __IMAGE_HPP__

#include "../rect2/rect2_t.hpp"
#include "../color/color_t.hpp"
#include "../rgb.h"
#include <functional>

template <typename ColorType>
class Painter;

template <typename ColorType>
class Image;

template <typename ColorType>
class ImageWithData;


template<typename ColorType>
struct PixelProxy{
public:
    ImageWithData<ColorType> & src;
    Vector2i pos;

    PixelProxy(ImageWithData<ColorType> & _src, Vector2i _pos) : src(_src), pos(_pos) {}

    auto & operator = (const ColorType & color){
        src.putPixel(pos, color);
        return *this;
    }

    operator ColorType () const{
        return src.getPixel(pos);
    }

};


template<typename ColorType>
class ImageBasics{
public:
    Rect2i area = Rect2i();
    ImageBasics(const Vector2i & size):area(Vector2i(), size){;}

    Rect2i getDisplayArea() const {
        return area;
    }

    
    Rect2i getArea() const{return area;}
    Vector2i getSize() const{return area.size;}

    bool hasPoint(const Vector2i & pos){
        return area.has_point(pos);
    }
};

template<typename ColorType>
class ImageReadable:virtual public ImageBasics<ColorType>{
protected:
    virtual void getPixel_Unsafe(const Vector2i & pos, ColorType & color) = 0;
    void getPixel(const Vector2i & pos, ColorType & color){
        if(this->hasPoint(pos)){
            getPixel_Unsafe(pos, color);
        }
    }

    virtual void putTexture_Unsafe(const Rect2i & rect, const ColorType * color_ptr) = 0;

    virtual void putRect_Unsafe(const Rect2i & rect, const ColorType & color) = 0;

public:
    // ImageReadable() = default;
    ImageReadable(const Vector2i & size):ImageBasics<ColorType>(size){;}
};

template<typename ColorType>
class ImageWritable:virtual public ImageBasics<ColorType>{
protected:
    virtual void setPosition_Unsafe(const Vector2i & pos) = 0;
    virtual void setArea_Unsafe(const Rect2i & rect) = 0;
    virtual void putPixel_Unsafe(const Vector2i & pos, const ColorType & color) = 0;
    void putPixel(const Vector2i & pos, const ColorType & color){
        if(this->hasPoint(pos)){
            getPixel_Unsafe(pos, color);
        }
    }

    virtual void putTexture_Unsafe(const Rect2i & rect, const ColorType * color_ptr) = 0;

    virtual void putRect_Unsafe(const Rect2i & rect, const ColorType & color) = 0;
public:
    // ImageWritable() = default;
    ImageWritable(const Vector2i & size):ImageBasics<ColorType>(size){;}
};

template<typename ColorType>
class Image:public ImageReadable<ColorType>, public ImageWritable<ColorType>{
protected:

    using PixelShaderCallback = ColorType(*)(const Vector2i &);
    using UVShaderCallback = ColorType(*)(const Vector2 &);

    friend class Painter<ColorType>;
    friend class PixelProxy<ColorType>;
public:
    Image(const Vector2i & size):ImageReadable<ColorType>(size), ImageWritable<ColorType>(size){;}
    void shade(PixelShaderCallback callback, const Rect2i & _shade_area);
    void shade(UVShaderCallback callback, const Rect2i & _shade_area);
};

template<typename ColorType>
class ImageWithData:public Image<ColorType>{
private:
    Rect2i select_area;
    bool removeable = false;
protected:
    ColorType * data = nullptr;

    void setPosition_Unsafe(const Vector2i & pos) override {select_area.position = pos;}
    void setArea_Unsafe(const Rect2i & rect) override {select_area = rect;}
    void putPixel_Unsafe(const Vector2i & pos, const ColorType & color) override{data[this->getArea().size.x * pos.y + pos.x] = color;}
    void getPixel_Unsafe(const Vector2i & pos, ColorType & color) override{color = data[this->getArea().size.x * pos.y + pos.x];}
    // void putTexture_Unsafe(const Rect2i & rect, const Grayscale * color_ptr)   override{

    // }
    void putTexture_Unsafe(const Rect2i & rect, const ColorType * color_ptr) override{
        setArea_Unsafe(rect);
        // Vector2i point = rect.position;
        uint32_t i = 0;
        for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++, i++)
                putPixel_Unsafe(Vector2i(x,y), color_ptr[i]);
    }

    void putRect_Unsafe(const Rect2i & rect, const ColorType & color) override{
        setArea_Unsafe(rect);
        for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++)
                putPixel_Unsafe(Vector2i(x,y), color);
    }



public:

    ImageWithData(ColorType * _data, const Vector2i & size): Image<ColorType>(size), data(_data){;}
    ImageWithData(const Vector2i & size): Image<ColorType>(size), data(new ColorType[size.x * size.y]){
        removeable = true;
    }

    ~ImageWithData(){
        if(removeable){
            delete[] data;
        }
    }
    ColorType * getData() override{return data;}
    ColorType & operator [](const size_t & index){return data[index];}

    __fast_inline ColorType & operator()(const Vector2i & pos){
        if(!this->area.has_point(pos)) return data[0];
        return data[pos.x + pos.y * this -> area.size.x];
    }

    __fast_inline ColorType & operator()(const int & x, const int & y){
        if(!this->area.has_point(Vector2i(x,y))) return data[0];
        return data[x + y * this -> area.size.x];
    }

    __fast_inline ColorType operator()(const Vector2 & pos);
    __fast_inline ColorType operator()(const real_t & x, const real_t & y);
    __fast_inline ColorType pick(const int & x, const int & y) const{return data[x + y * this -> area.size.x];}

    Vector2i uv2pixel(const Vector2 & uv) const{
        return Vector2i(int(LERP((uv.x + 1) / 2, 0, this->area.size.x)), int(LERP((uv.y + 1) / 2, 0, this->area.size.y)));
    }

    Vector2 uv2aero(const Vector2 & uv) const{
        return Vector2(((uv.x + 1) * (this->area.size.x / 2)), (uv.y + 1) * (this->area.size.y / 2));
    }

    Vector2 pixel2uv(const Vector2i & pixel)const{
        return Vector2(INVLERP((real_t)pixel.x, this->area.size.x / 2, this->area.size.x), INVLERP((real_t)pixel.y, this->area.size.y / 2, this->area.size.y));
    }

    Vector2 uvstep(){
        return Vector2(real_t(2) / this->area.size.x, real_t(2) / this->area.size.y);
    }

    uint8_t adaptiveThreshold(const Vector2i & pos){
        auto & x = pos.x;
        auto & y = pos.y;
        uint16_t average=0;
        int i,j;
        for(i=y-3;i<=y+3;i++)
        {
            for(j=x-3;j<=x+3;j++)
            {
                average += uint8_t(operator()(j, i));
            }
        }
        average=average/49-23;
        if(uint8_t(operator()(x, y)) > average)
            return 255;
        else return 0;
    }
};

#include "Image.tpp"

class Image565 : public Image<RGB565>{

};
#endif