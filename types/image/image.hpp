#ifndef __IMAGE_HPP__

#define __IMAGE_HPP__

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
            int cols;
            int rows;
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

    virtual Rect2i get_window() const{
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

    __fast_inline ColorType operator()(const Vector2i & pos)const{
        ColorType color;
        getpixel(pos, color);
        return color;
    }

    __fast_inline ColorType operator()(const size_t & index)const{
        ColorType color;
        getpixel(Vector2i(index % ImageBasics::get_size().x, index / ImageBasics::get_size().x), color);
        return color;
    }

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
public:
    virtual void setpos_unsafe(const Vector2i & pos) = 0;
    virtual void setarea_unsafe(const Rect2i & rect) = 0;
    virtual void putpixel_unsafe(const Vector2i & pos, const ColorType & color) = 0;

    // template<U>
    // void putpixel_unsafe(const Vector2i & pos, const Grs & color){
    //     putpixel_unsafe(pos, RGB::conv());
    // }
    virtual void puttexture_unsafe(const Rect2i & rect, const ColorType * color_ptr){
        setarea_unsafe(rect);
        uint32_t i = 0;
        for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++, i++)
                putpixel_unsafe(Vector2i(x,y), color_ptr[i]);
    }

    // virtual void puttexture_unsafe(const Rect2i & rect, const ColorType * color_ptr){
    //     setarea_unsafe(rect);
    //     uint32_t i = 0;
    //     for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
    //         for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++, i++)
    //             putpixel_unsafe(Vector2i(x,y), color_ptr[i]);
    // }

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
            if(bool(area) == false) return;
            setarea_unsafe(area);
            uint32_t i = 0;
            for(int x = area.position.x; x < area.position.x + area.size.x; x++)
                for(int y = area.position.y; y < area.position.y + area.size.y; y++, i++)
                    putpixel_unsafe(Vector2i(x,y), color_ptr[i]);
        }
    }
public:
    ImageWritable(const Vector2i & _size):ImageBasics(_size){;}

    void fill(const ColorType & color){
        putrect_unsafe(Rect2i{{}, ImageBasics::get_size()}, color);
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

    __fast_inline PixelProxy<ColorType> operator[](const Vector2i & pos){
        return PixelProxy<ColorType>(*this,pos);
    }

    __fast_inline PixelProxy<ColorType> operator[](const size_t & index){
        return PixelProxy<ColorType>(*this,Vector2i(index % ImageBasics::get_size().x, index / ImageBasics::get_size().x));
    }
};



template<typename ColorType>
class ImageWR:public ImageReadable<ColorType>, public ImageWritable<ColorType>{
protected:


    using PixelShaderCallback = ColorType(*)(const Vector2i &);
    using UVShaderCallback = ColorType(*)(const Vector2 &);

    friend class Painter<ColorType>;
    // friend class PixelProxy<ColorType>;
public:
    ImageWR(const Vector2i & _size):ImageReadable<ColorType>(_size), ImageWritable<ColorType>(_size){;}
    // void shade(PixelShaderCallback callback, const Rect2i & _shade_area);
    // void shade(UVShaderCallback callback, const Rect2i & _shade_area);
};



template<typename ColorType, typename DataType>
class ImageWithData : public ImageWR<ColorType> {
protected:
    Rect2i select_area;

public:
    void setpos_unsafe(const Vector2i & pos) override { select_area.position = pos; }
    void setarea_unsafe(const Rect2i & rect) override { select_area = rect; }
    void putpixel_unsafe(const Vector2i & pos, const ColorType & color) override { data[this->size.x * pos.y + pos.x] = color; }
    void getpixel_unsafe(const Vector2i & pos, ColorType & color) const override { color = data[this->size.x * pos.y + pos.x]; }

public:
    std::shared_ptr<DataType[]> data;
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


    __fast_inline const DataType operator()(const size_t & index) const {return data[index]; }
    __fast_inline const ColorType operator()(const Vector2i & pos) const {return ImageBasics::get_size().has_point(pos) ? data[pos.x + pos.y * ImageBasics::get_size().x] : ColorType(0);}

    __fast_inline const DataType& operator[](const size_t & index) const { return data[index]; }
    __fast_inline const ColorType& operator[](const Vector2i & pos) const { return data[pos.x + pos.y * ImageBasics::w]; }

    __fast_inline DataType& operator[](const size_t & index) { return data[index]; }
    __fast_inline ColorType& operator[](const Vector2i & pos) { return data[pos.x + pos.y * ImageBasics::w]; }


    template<typename ToColorType>
    __fast_inline ToColorType at(const int y, const int x) const { return data[x + y * ImageBasics::w]; }

    // template<>
    __fast_inline ColorType & at(const int y, const int x){ return data[x + y * ImageBasics::w]; }


    bool operator == (const ImageWithData<auto, auto> & other) const {
        return data == other.data;
    }
};


template<typename ColorType>
class Image:public ImageWithData<ColorType, ColorType>{
public:
    Image(std::shared_ptr<ColorType[]> _data, const Vector2i & _size):  ImageBasics(_size), ImageWithData<ColorType, ColorType>(_data, _size) {}

    Image(const Vector2i & _size): ImageBasics(_size), ImageWithData<ColorType, ColorType>(_size) {}

    Image(Image&& other) noexcept : ImageBasics(other.get_size()),  ImageWithData<ColorType, ColorType>(std::move(other)){;}

    Image(const Image & other) noexcept: ImageBasics(other.get_size()),  ImageWithData<ColorType, ColorType>(other) {}

    Image & operator=(Image && other) noexcept {
        if (this != &other) {
            this->size = std::move(other.size);
            this->select_area = std::move(other.select_area);
            this->data = std::move(other.data);
        }
        return *this;
    }

    Image<ColorType> clone() const {
        auto _size = ImageBasics::get_size();
        auto temp = Image<ColorType>(_size);
        memcpy(temp.data.get(), this->data.get(), _size.x * _size.y * sizeof(ColorType));
        return temp;
    }

    Image<ColorType> & clone(const Image<ColorType> & other){
        auto _size = (Rect2i(ImageBasics::get_size())).intersection(Rect2i(other.get_size())).size;
        this->size = _size;
        this->data = std::make_shared<ColorType[]>(_size.x * _size.y);
        memcpy(this->data.get(), other.data.get(), _size.x * _size.y * sizeof(ColorType));
        return *this;
    }

    Image<ColorType> clone(const Rect2i & rect) const {
        auto temp = Image<ColorType>(rect.size);
        for(int j = 0; j < rect.h; j++) {
            for(int i = 0; i < rect.w; i++) {
                temp[Vector2i{i,j}] = this->operator[](Vector2i{i + rect.x, j + rect.y});
            }
        }
        return temp;
    }

    auto clone(const Vector2i & _size) const{return clone(Rect2i(Vector2i{0,0}, _size));}

    Grayscale mean(const Rect2i & roi) const;

    uint64_t sum(const Rect2i & roi) const;
    uint64_t sum() const{return sum(this->get_window());}

    Grayscale mean() const{return mean(this->get_window());}
    void load(const uint8_t * buf, const Vector2i & _size);
    static Image<ColorType> load_from_buf(const uint8_t * buf, const Vector2i & _size){
        Image<ColorType> img(_size);
        img.load(buf, _size);
        return img;
    }

    template<typename toColorType = ColorType>
    Image<toColorType> space() const {
        return Image<toColorType>(this->get_size());
    }
};



template<typename ColorType, typename DataType>
class ImageDataTypeDiff:public Image<ColorType>{

};


template<typename ColorType>
class ImageView:public ImageReadable<ColorType>, public ImageWritable<ColorType>{
protected:
    using m_Image = ImageWR<ColorType>;
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
class Camera:public Image<ColorType>{
public:
    Camera(const Vector2i & _size):ImageBasics(_size), Image<ColorType>(_size){;}
};

template<typename ColorType>
class Displayer:public ImageWritable<ColorType>{
public:
    Displayer(const Vector2i & _size):ImageBasics(_size), ImageWritable<ColorType>(_size){;}
};

template<typename ColorType>
struct PixelProxy{
public:
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


#include "Image.tpp"


template<typename ColorType>
__inline auto make_image(const Vector2i & _size){
    return Image<ColorType>(_size);
}

__inline auto make_gray(const Vector2i & _size){return make_image<Grayscale>(_size);};
__inline auto make_bina(const Vector2i & _size){return make_image<Binary>(_size);};


template<typename ColorType>
__inline auto make_mirror(const Image<auto> &src){
    return Image<ColorType>(std::reinterpret_pointer_cast<ColorType[]>(src.data),
        src.get_size());
}


template<typename ColorType>
__inline auto make_gray_mirror(const Image<ColorType> & src){return make_mirror<Grayscale>(src);};

template<typename ColorType>
__inline auto make_bina_mirror(const Image<ColorType> & src){return make_mirror<Binary>(src);};

#endif