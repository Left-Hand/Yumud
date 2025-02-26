#pragma once

#include "sys/core/platform.h"
#include "ImageBasics.hpp"


namespace ymd{

template<typename ColorType>
class Image:public ImageWithData<ColorType, ColorType>{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
public:
    auto get_data() const {return this->data.get();}
    auto get_ptr() const {return this->data;}
    Image(std::shared_ptr<ColorType[]> _data, const Vector2i & _size):  ImageBasics(_size), ImageWithData<ColorType, ColorType>(_data, _size) {}

    Image(const Vector2i & _size): ImageBasics(_size), ImageWithData<ColorType, ColorType>(_size) {}

    Image(Image&& other) noexcept : ImageBasics(other.size()),  ImageWithData<ColorType, ColorType>(std::move(other)){;}

    Image(const Image & other) noexcept: ImageBasics(other.size()),  ImageWithData<ColorType, ColorType>(other) {}
 
    Image & operator=(Image && other) noexcept {
        if (this != &other) {
            this->size_ = std::move(other.size());
            this->select_area = std::move(other.select_area);
            this->data = std::move(other.data);
        }
        return *this;
    }

    Image<ColorType> clone() const {
        auto temp = Image<ColorType>(this->size());
        memcpy(temp.data.get(), this->data.get(), this->size().x * this->size().y * sizeof(ColorType));
        return temp;
    }

    Image<ColorType> & clone(const Image<ColorType> & other){
        const auto _size = (Rect2i(ImageBasics::size())).intersection(Rect2i(other.size())).size;
        this->size_ = _size;
        this->data = std::make_shared<ColorType[]>(_size.x * _size.y);
        memcpy(this->data.get(), other.data.get(), _size.x * _size.y * sizeof(ColorType));
        return *this;
    }

    Image<ColorType> clone(const Rect2i & view) const {
        auto temp = Image<ColorType>(view.size);
        for(int j = 0; j < view.h; j++) {
            for(int i = 0; i < view.w; i++) {
                temp[Vector2i{i,j}] = this->operator[](Vector2i{i + view.x, j + view.y});
            }
        }
        return temp;
    }

    auto clone(const Vector2i & _size) const{return clone(Rect2i(Vector2i{0,0}, _size));}

    Grayscale mean(const Rect2i & view) const;
    Grayscale mean() const{return mean(this->rect());}

    uint64_t sum(const Rect2i & roi) const;
    uint64_t sum() const{return sum(this->rect());}
    Grayscale bilinear_interpol(const Vector2 & pos) const;

    void load(const uint8_t * buf, const Vector2i & _size);
    static Image<ColorType> load_from_buf(const uint8_t * buf, const Vector2i & _size){
        Image<ColorType> img(_size);
        img.load(buf, _size);
        return img;
    }

    template<typename toColorType = ColorType>
    Image<toColorType> space() const {
        return Image<toColorType>(this->size());
    }
};


template<typename ColorType, typename DataType>
class ImageDataTypeDiff:public Image<ColorType>{

};


template<typename ColorType>
class ImageView:public ImageReadable<ColorType>, public ImageWritable<ColorType>{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
protected:
    using m_Image = ImageWR<ColorType>;
    m_Image & instance;
    Rect2i window;
public:
    ImageView(m_Image & _instance):instance(_instance){}
    ImageView(m_Image & _instance, const Rect2i & _window):instance(_instance), window(_window){;}

    ImageView(ImageView & other, const Rect2i & _window):instance(other.instance), 
        window(Rect2i(other.window.position + _window.position, other.window.size).intersection(Vector2i(), other.instance.getSize())){;}
    Rect2i rect() const {return window;}
};

template<typename ColorType>
class Camera:public Image<ColorType>{
public:
    using Vector2 = Image<ColorType>::Vector2;
    using Vector2i = Image<ColorType>::Vector2i;
    Camera(const Vector2i & _size):ImageBasics(_size), Image<ColorType>(_size){;}
};

template<typename ColorType>
class Displayer:public ImageWritable<ColorType>{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
public:
    Displayer(const Vector2i & size):ImageBasics(size), ImageWritable<ColorType>(size){;}
};

template<typename ColorType>
struct PixelProxy{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
    
    ImageWritable<ColorType> & src;
    Vector2i pos;

    PixelProxy(ImageWritable<ColorType> & _src, Vector2i _pos) : src(_src), pos(_pos) {}

    auto & operator = (const ColorType & color){
        src.putpixel(pos, color);
        return *this;
    }

    operator ColorType () const{
        return src.getpixel(pos);
    }
};



template<typename ColorType>
__inline auto make_image(const ImageBasics::Vector2i & _size){
    return Image<ColorType>(_size);
}

__inline auto make_gray(const ImageBasics::Vector2i & _size){return make_image<Grayscale>(_size);};
__inline auto make_bina(const ImageBasics::Vector2i & _size){return make_image<Binary>(_size);};


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

