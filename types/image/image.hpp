#pragma once

#include "core/platform.hpp"
#include "ImageBasics.hpp"


namespace ymd{

template<typename ColorType>
class Image:public ImageWithData<ColorType, ColorType>{
public:


    Image(std::shared_ptr<ColorType[]> _data, const Vector2u & _size): 
        ImageWithData<ColorType, ColorType>(_data, _size) {}

    Image(const Vector2u & _size): 
        ImageWithData<ColorType, ColorType>(_size) {}

    Image(Image&& other) noexcept : 
        ImageWithData<ColorType, ColorType>(std::move(other)){;}

    Image(const Image & other) noexcept: 
        ImageWithData<ColorType, ColorType>(other) {}


    Image & operator=(Image&& other){
        return *this;
    }
    Image & operator=(const Image& other){
        return *this;
    }

    Image<ColorType> clone() const {
        const auto img_size = this->size();
        auto temp = Image<ColorType>(img_size);
        memcpy(
            temp.get_data(), 
            this->get_data(), 
            img_size.x * img_size.y * sizeof(ColorType)
        );
        return temp;
    }

    // Image<ColorType> & clone(const Image<ColorType> & other){
    //     const auto size = ImageBasics::size().overlap_as_vec2(other.size());
    //     this-> set_size(size);
    //     this->data_ = std::make_shared<ColorType[]>(size.area());
    //     memcpy(this->data_.get(), other.data_.get(), size.area() * sizeof(ColorType));
    //     return *this;
    // }

    Image<ColorType> clone(const Rect2u & view) const {
        auto temp = Image<ColorType>(view.size);
        for(size_t j = 0; j < view.h(); j++) {
            for(size_t i = 0; i < view.w(); i++) {
                temp[Vector2u{i,j}] = this->operator[](Vector2u{i + view.x(), j + view.y()});
            }
        }
        return temp;
    }

    auto clone(const Vector2u & _size) const{return clone(Rect2u(Vector2u{0,0}, _size));}

    constexpr ColorType mean(const Rect2u & view) const;
    constexpr ColorType mean() const{return mean(this->size().to_rect());}

    constexpr uint64_t sum(const Rect2u & roi) const;
    constexpr uint64_t sum() const{return sum(this->size().to_rect());}
    constexpr ColorType bilinear_interpol(const Vector2q<16> & pos) const;

    static Image<ColorType> load_from_buf(const uint8_t * buf, const Vector2u & _size){
        auto data = std::make_shared<ColorType[]>(_size.x * _size.y);
        memcpy(data.get(), buf, _size.x * _size.y * sizeof(ColorType));
        return Image<ColorType>(std::move(data), _size);
    }

    template<typename toColorType = ColorType>
    Image<toColorType> space() const {
        return Image<toColorType>(this->size());
    }

    void set_pixel(const Vector2u & pos, const ColorType color) {
        this->data_[pos.y * this->size().x + pos.x] = color;
    }
};


template<typename ColorType, typename DataType>
class ImageDataTypeDiff:public Image<ColorType>{

};


template<typename ColorType>
class ImageView:
    public ImageReadableIntf<ColorType>, 
    public ImageWritableIntf<ColorType>{
public:
protected:
    using m_Image = ImageWR<ColorType>;
    m_Image & instance;
    Rect2u window;
public:
    ImageView(m_Image & _instance):instance(_instance){}
    ImageView(m_Image & _instance, const Rect2u & _window):
        instance(_instance), window(_window){;}

    ImageView(ImageView & other, const Rect2u & _window):
        instance(other.instance), 
        window(Rect2u(other.window.position + _window.position, other.window.size)
        .intersection(Vector2u(), other.instance.get_size())){;}
    Rect2u rect() const {return window;}
};

template<typename ColorType>
struct PixelProxy{
public:
    
    ImageWritableIntf<ColorType> & src;
    Vector2u pos;

    PixelProxy(ImageWritableIntf<ColorType> & _src, Vector2u _pos) : src(_src), pos(_pos) {}

    auto & operator = (const ColorType & color){
        src.putpixel(pos, color);
        return *this;
    }

    operator ColorType () const{
        return src.getpixel(pos);
    }
};



template<typename ColorType>
__inline auto make_image(const Vector2u & _size){
    return Image<ColorType>(_size);
}

__inline auto make_gray(const Vector2u & _size){return make_image<Grayscale>(_size);};
__inline auto make_bina(const Vector2u & _size){return make_image<Binary>(_size);};


template<typename ColorType>
__inline auto make_mirror(const Image<auto> &src){
    return Image<ColorType>(std::reinterpret_pointer_cast<ColorType[]>(src.get_ptr()),
        src.size());
}


template<typename ColorType>
__inline auto make_gray_mirror(const Image<ColorType> & src){return make_mirror<Grayscale>(src);};

template<typename ColorType>
__inline auto make_bina_mirror(const Image<ColorType> & src){return make_mirror<Binary>(src);};


}

#include "image.tpp"

