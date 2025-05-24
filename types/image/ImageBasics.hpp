#pragma once


#include "types/vectors/vector2/vector2.hpp"
#include "types/regions/rect2/rect2.hpp"
#include "types/colors/color/color.hpp"
#include "types/colors/rgb/rgb.hpp"
#include <functional>
#include <memory>


namespace ymd{

    // class Painter;
    
template <typename ColorType>
class Painter;

template <typename ColorType>
class ImageWR;


template <typename ColorType>
class PixelProxy;



class ImageBasics{
private:
    Vector2u size_;
protected:
    __fast_inline void set_size(const Vector2u size){
        size_ = size;
    }
public:

    ImageBasics(const Vector2u & size):size_(size){;}

    Vector2u uv2pixel(const Vector2q<16> & uv) const{
        return Vector2u(
            int(LERP(0u, this->size().x, ((uv.x + 1) / 2))), 
            int(LERP(0u, this->size().y, (uv.y + 1)/2)));
    }

    Vector2q<16> uv2aero(const Vector2q<16> & uv) const{
        return Vector2q<16>(((uv.x + 1) * (this->size().x / 2)), (uv.y + 1) * (this->size().y / 2));
    }

    Vector2q<16> pixel2uv(const Vector2u & pixel) const {
        return Vector2q<16>(
            INVLERP(this->size().x / 2, this->size().x, real_t(pixel.x)), 
            INVLERP(this->size().y / 2, this->size().y, real_t(pixel.y)));
    }

    Vector2q<16> uvstep() const{
        return Vector2q<16>(real_t(2) / this->size().x, real_t(2) / this->size().y);
    }

    __fast_inline Vector2u size() const{
        return this->size_;
    }


};

template<typename ColorType>
class ImageReadable:virtual public ImageBasics{
protected:
    virtual void getpixel_unsafe(const Vector2u & pos, ColorType & color) const = 0;
    void getpixel(const Vector2u & pos, ColorType & color) const{
        if(this->size().has_point(pos)){
            getpixel_unsafe(pos, color);
        }
    }

    uint8_t getseg_v8() const{return 0;}
    uint8_t getseg_h8() const{return 0;}
public:
    ImageReadable(const Vector2u & _size):ImageBasics(_size){;}

    __fast_inline ColorType operator[](const Vector2u & pos)const{
        ColorType color;
        getpixel(pos, color);
        return color;
    }

    __fast_inline ColorType operator[](const size_t index)const{
        ColorType color;
        getpixel(Vector2u(index % ImageBasics::size().x, index / ImageBasics::size().x), color);
        return color;
    }

    __fast_inline ColorType at(const size_t y, const size_t x)const{
        ColorType color;
        getpixel({x,y}, color);
        return color;
    }

};



template<typename ColorType>
class ImageWritable:virtual public ImageBasics{
protected:

    virtual void setpos_unsafe(const Vector2u & pos) = 0;
    virtual void setarea_unsafe(const Rect2u & rect) = 0;
    virtual void putpixel_unsafe(const Vector2u & pos, const ColorType color) = 0;

    virtual void puttexture_unsafe(const Rect2u & rect, const ColorType * color_ptr){
        setarea_unsafe(rect);
        uint32_t i = 0;
        for(size_t x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(size_t y = rect.position.y; y < rect.position.y + rect.size.y; y++, i++)
                putpixel_unsafe(Vector2u(x,y), color_ptr[i]);
    }

    virtual void putrect_unsafe(const Rect2u & rect, const ColorType color){
        
        setarea_unsafe(rect);
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
public:
    ImageWritable(const Vector2u & _size):ImageBasics(_size){;}

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

    __fast_inline PixelProxy<ColorType> operator[](const size_t index){
        return PixelProxy<ColorType>(*this,Vector2u(
            index % ImageBasics::size().x, 
            index / ImageBasics::size().x)
        );
    }
};


template<typename ColorType>
class ImageWR:public ImageReadable<ColorType>, public ImageWritable<ColorType>{
public:
protected:
    using PixelShaderCallback = ColorType(*)(const Vector2u &);
    using UVShaderCallback = ColorType(*)(const Vector2q<16> &);

    friend class Painter<ColorType>;
    // friend class Painter;
public:
    ImageWR(const Vector2u & _size):
        ImageReadable<ColorType>(_size), 
        ImageWritable<ColorType>(_size){;}
};


template<typename ColorType, typename DataType>
class ImageWithData : public ImageWR<ColorType> {
protected:
    void setpos_unsafe(const Vector2u & pos) {;}

    void setarea_unsafe(const Rect2u & rect) {;}

    void putpixel_unsafe(const Vector2u & pos, const ColorType color) 
        { data_[this->size().x * pos.y + pos.x] = color; }
    void getpixel_unsafe(const Vector2u & pos, ColorType & color) const 
        { color = data_[this->size().x * pos.y + pos.x]; }

    std::shared_ptr<DataType[]> data_;
public:
    ImageWithData(std::shared_ptr<DataType[]> _data, const Vector2u & _size) : 
        ImageBasics(_size), ImageWR<ColorType>(_size), data_(_data) {;}

    ImageWithData(const Vector2u & _size) : 
        ImageBasics(_size), ImageWR<ColorType>(_size), 
        data_(std::make_shared<DataType[]>(_size.x * _size.y)) {;}

    // Move constructor
    ImageWithData(ImageWithData&& other) noexcept : 
        ImageBasics(other.size()), 
        ImageWR<ColorType>(other.size()), data_(std::move(other.data_)){}


    ImageWithData(const ImageWithData& other) noexcept : 
        ImageBasics(other.size()), 
        ImageWR<ColorType>(other.size()), data_(other.data_){}

    ImageWithData& operator=(ImageWithData&& other) noexcept {
        if (this != &other) {
            this->size_ = std::move(other.size_);
            this->data_ = std::move(other.data_);
        }
        return *this;
    }


    __fast_inline const DataType & operator[](const size_t index) const { 
        return data_[index]; }

    __fast_inline DataType & operator[](const size_t index) {
        return data_[index]; }

    __fast_inline ColorType & operator[](const Vector2u & pos) {
        return data_[pos.x + pos.y * this->size().x]; }

    __fast_inline const ColorType & operator[](const Vector2u & pos) const {
        return data_[pos.x + pos.y * this->size().x]; }


    template<typename ToColorType>
    __fast_inline ToColorType at(const int y, const int x) const {
        return data_[x + y * this->size().x]; }

    __fast_inline ColorType & at(const int y, const int x){
        return data_[x + y * this->size().x]; }

    __fast_inline const ColorType & at(const int y, const int x)const{
        return data_[x + y * this->size().x]; }


    bool operator == (const ImageWithData<auto, auto> & other) const {
        return data_ == other.data_;
    }
};

}