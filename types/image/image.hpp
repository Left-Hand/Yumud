#ifndef __IMAGE_HPP__

#define __IMAGE_HPP__

#include "types/rect2/rect2_t.hpp"
#include "types/color/color_t.hpp"
#include "types/rgb.h"
#include <functional>

template <typename ColorType>
class Painter;

template <typename ColorType>
class Image;

template <typename ColorType, typename DataType>
class ImageWithData;


// template<typename ColorType>
// struct PixelProxy{
// public:
//     ImageWithData<ColorType> & src;
//     Vector2i pos;

//     PixelProxy(ImageWithData<ColorType> & _src, Vector2i _pos) : src(_src), pos(_pos) {}

//     auto & operator = (const ColorType & color){
//         src.putpixel(pos, color);
//         return *this;
//     }

//     operator ColorType () const{
//         return src.get_pixel(pos);
//     }

// };


template<typename ColorType>
class ImageBasics{
public:
    Vector2i size;

    ImageBasics(const Vector2i & _size):size(_size){;}

    // Rect2i getDisplayArea() const {
    //     return area;
    // }

    Vector2i getSize() const{return size;}

    // bool has_point(const Vector2i & pos){
    //     return area.has_point(pos);
    // }
    bool has_point(const Vector2i & pos){
        return size.has_point(pos);
    }

    virtual Rect2i get_window() const{
        return Rect2i({}, size);
    }
};

template<typename ColorType>
class ImageReadable:virtual public ImageBasics<ColorType>{
protected:
    virtual void getpixel_unsafe(const Vector2i & pos, ColorType & color) = 0;
    void get_pixel(const Vector2i & pos, ColorType & color){
        if(this->has_point(pos)){
            getpixel_unsafe(pos, color);
        }
    }

    uint8_t getsegv8() const{return 0;}
    uint8_t getsegh8() const{return 0;}
public:
    ImageReadable(const Vector2i & size):ImageBasics<ColorType>(size){;}
};

template<typename ColorType>
class ImageWritable:virtual public ImageBasics<ColorType>{
// protected:
public:
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

    void putrect(const Rect2i & rect, const ColorType & color){
        auto area = rect.intersection(this->get_window());
        putrect_unsafe(area, color);
    }

    void puttexture(const Rect2i & rect, const ColorType * color_ptr){
        if(rect.inside(this->get_window())){
            puttexture_unsafe(rect, color_ptr);
        }else{
            auto area = rect.intersection(this->get_window());
            setarea(area);
            uint32_t i = 0;
            for(int x = area.position.x; x < area.position.x + area.size.x; x++)
                for(int y = area.position.y; y < area.position.y + area.size.y; y++, i++)
                    putpixel_unsafe(Vector2i(x,y), color_ptr[i]);
        }
    }
public:
    ImageWritable(const Vector2i & size):ImageBasics<ColorType>(size){;}
    virtual void putseg_v8_unsafe(const Vector2i & pos, const uint8_t & mask, const ColorType & color){
        Rect2i area(pos, Vector2i(1, 8));
        if(Rect2i(this->size, Vector2i()).contains(area)){
            for(int i = 0; i < 8; i++){
                if(mask & (1 << i)) putpixel_unsafe(pos + Vector2i(0, i), color);
            }
        }else{
            for(int i = 0; i < 8; i++){
                if(mask & (1 << i))putpixel(pos + Vector2i(0, i), color);
            }
        }
    }

    virtual void putseg_h8_unsafe(const Vector2i & pos, const uint8_t & mask, const ColorType & color){
        Rect2i area(pos, Vector2i(8, 1));
        if(Rect2i(this->size, Vector2i()).contains(area)){
            for(int i = 0; i < 8; i++){
                if(mask & (0x80 >> i))putpixel_unsafe(pos + Vector2i(i, 0), color);
            }
        }else{
            for(int i = 0; i < 8; i++){
                if(mask & (1 << i))putpixel(pos + Vector2i(i, 0), color);
            }
        }
    }

};



template<typename ColorType>
class Image:public ImageReadable<ColorType>, public ImageWritable<ColorType>{
protected:


    using PixelShaderCallback = ColorType(*)(const Vector2i &);
    using UVShaderCallback = ColorType(*)(const Vector2 &);

    friend class Painter<ColorType>;
    // friend class PixelProxy<ColorType>;
public:
    Image(const Vector2i & size):ImageReadable<ColorType>(size), ImageWritable<ColorType>(size){;}
    // void shade(PixelShaderCallback callback, const Rect2i & _shade_area);
    // void shade(UVShaderCallback callback, const Rect2i & _shade_area);
};

template<typename ColorType, typename DataType>
class ImageWithData:public Image<ColorType>{
protected:
    Rect2i select_area;
    bool removeable = false;

public:
    void setpos_unsafe(const Vector2i & pos) override {select_area.position = pos;}
    void setarea_unsafe(const Rect2i & rect) override {select_area = rect;}
    void putpixel_unsafe(const Vector2i & pos, const ColorType & color) override{data[this->size.x * pos.y + pos.x] = color;}
    void getpixel_unsafe(const Vector2i & pos, ColorType & color) override{color = data[this->size.x * pos.y + pos.x];}


public:
    DataType * data = nullptr;
    ImageWithData(DataType * _data, const Vector2i & size): ImageBasics<ColorType>(size), Image<ColorType>(size), data(_data){;}
    ImageWithData(const Vector2i & size): Image<ColorType>(size), data(new ColorType[size.x * size.y]){
        removeable = true;
    }

    ~ImageWithData(){
        if(removeable){
            delete[] data;
        }
    }
    DataType * getData() {return data;}
    DataType & operator [](const size_t & index){return data[index];}

    __fast_inline DataType & operator()(const Vector2i & pos){
        if(!this->area.has_point(pos)) return data[0];
        return data[pos.x + pos.y * this -> area.size.x];
    }

    __fast_inline DataType & operator()(const int & x, const int & y){
        if(!this->area.has_point(Vector2i(x,y))) return data[0];
        return data[x + y * this -> area.size.x];
    }

    __fast_inline DataType operator()(const Vector2 & pos);
    __fast_inline DataType operator()(const real_t & x, const real_t & y);
    __fast_inline DataType pick(const int & x, const int & y) const{return data[x + y * this -> area.size.x];}

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


template<typename ColorType>
class ImageView:public ImageReadable<ColorType>, public ImageWritable<ColorType>{
protected:
    using m_Image = Image<ColorType>;
    m_Image & instance;
    Rect2i window;
public:
    ImageView(m_Image & _instance):instance(_instance){}
    ImageView(m_Image & _instance, const Rect2i & _window):instance(_instance), window(_window){;}

    ImageView(ImageView & other, const Rect2i & _window):instance(other.instance), 
        window(Rect2i(other.window.position + _window.position, other.window.size).intersection(Vector2i(), other.instance.getSize())){;}
    Rect2i get_window() const {return window;}
};

template<typename ColorType>
class Camera:public ImageReadable<ColorType>{

};

template<typename ColorType>
class Displayer:public ImageWritable<ColorType>{
public:
    Displayer(const Vector2i & size):ImageBasics<ColorType>(size), ImageWritable<ColorType>(size){;}
};



#include "Image.tpp"

class Image565 : public Image<RGB565>{

};
#endif