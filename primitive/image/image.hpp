#pragma once


#include <functional>
#include <memory>

#include "core/utils/Result.hpp"

#include "algebra/vectors/vec2.hpp"
#include "algebra/regions/rect2.hpp"
#include "primitive/colors/color/color.hpp"
#include "primitive/colors/rgb/rgb.hpp"



namespace ymd{

// class Painter;
    
template <typename ColorType>
class Painter;

template <typename ColorType>
class ImageWR;


template <typename ColorType>
class PixelProxy;




template<typename T>
class [[nodiscard]] Image final{
public:
    enum class [[nodiscard]] Error{
        OutOfMemory,
        EmptySize,
        LoadFromNull,
        RegionOutOfSize,
    };

    template<typename U = void>
    using IResult = Result<U, Error>;

    explicit Image(const Vec2u size):
        size_(size),
        resource_(std::make_shared<T[]>(size.x * size.y)){
            ASSERT(not size.is_zero());
        }

    // IResult<Image> from_size(const Vec2u size){
    //     auto raw = new (std::nothrow) T[size.x * size.y];
    //     if (!raw) {
    //         return Err(Error::OutOfMemory);
    //     }
    //     return Ok(std::move(Image(
    //         std::shared_ptr<T[]>(raw, std::default_delete<T[]>()),
    //         size))
    //     );
    // }

    explicit Image(std::shared_ptr<T[]> data, const Vec2u size):
        size_(size),
        resource_(data){
            ASSERT(not size.is_zero());
        }


    [[nodiscard]] static Image<T> from_buf(const T * buf, const Vec2u size){
        auto data = std::make_shared<T[]>(size.x * size.y);
        for(size_t i = 0; i < size.x * size.y; i++){
            data.get()[i] = buf[i];
        }
        return Image<T>(std::move(data), size);
    }

    Image(const Image & other) = delete;
    
    Image& operator=(Image&& other) noexcept {
        if (not this->is_shared_with(other)) {
            this->size_ = std::move(other.size_);
            this->resource_ = std::move(other.resource_);
            other.size_ = Vec2u{0,0};
            other.resource_ = nullptr;
        }
        return *this;
    }
    
    Image(Image && other){
        this->size_ = other.size_;
        this->resource_ = std::move(other.resource_);
    }

    Image & operator=(const Image& other) = delete;
    bool operator ==(const Image & other) const = delete;

    [[nodiscard]] Image<T> clone() const {
        const auto img_size = this->size();
        auto temp = Image<T>(img_size);

        for(size_t i = 0; i < img_size.x * img_size.y; i++){
            temp.head_ptr()[i] = this->head_ptr()[i];
        }
        return temp;
    }

    [[nodiscard]] Image<T> clone(const Rect2u & view) const {
        auto temp = Image<T>(view.size);
        for(size_t j = 0; j < view.h(); j++) {
            for(size_t i = 0; i < view.w(); i++) {
                temp[Vec2u{i,j}] = this->operator[](Vec2u{i + view.x(), j + view.y()});
            }
        }
        return temp;
    }

    template<typename ColorType2> 
    requires (sizeof(T) == sizeof(ColorType2))
    [[nodiscard]] Image<ColorType2> shallow_copy(){
        return Image<ColorType2>(
            std::reinterpret_pointer_cast<T[]>(this->resource()),
            this->size());
    }

    [[nodiscard]] constexpr bool is_shared_with(const Image<auto> & other) const {
        return resource_ == other.resource_;
    }

    void putpixel(const Vec2u pos, const T color) {
        assert_position_is_inrange(pos);
        putpixel_unchecked(pos, color);
    }

    [[nodiscard]] __fast_inline const T & operator[](const size_t index) const { 
        return resource_[index];
    }

    [[nodiscard]] __fast_inline T & operator[](const size_t index) {
        return resource_[index]; }

    [[nodiscard]] __fast_inline T & operator[](const Vec2u pos) {
        return resource_[pos.x + pos.y * this->size().x]; }

    [[nodiscard]] __fast_inline const T & operator[](const Vec2u pos) const {
        return resource_[pos.x + pos.y * this->size().x]; }


    template<typename ToColorType>
    [[nodiscard]] __fast_inline ToColorType at(const Vec2u pos) const {
        assert_position_is_inrange(pos);
        return resource_[pos.x + pos.y * this->size().x]; }

    [[nodiscard]] __fast_inline T & at(const Vec2u pos){
        assert_position_is_inrange(pos);
        return resource_[pos.x + pos.y * this->size().x]; }

    [[nodiscard]] __fast_inline const T & at(const Vec2u pos)const{
        assert_position_is_inrange(pos);
        return resource_[pos.x + pos.y * this->size().x]; }

    void putpixel_unchecked(const Vec2u pos, const T color) 
        { resource_[this->size().x * pos.y + pos.x] = color; }
    void getpixel_unchecked(const Vec2u pos, T & color) const 
        { color = resource_[this->size().x * pos.y + pos.x]; }

private:


    Vec2u size_ = Vec2u::ZERO;

    std::shared_ptr<T[]> resource_;

    __fast_inline void assert_position_is_inrange(const Vec2u pos){
        ASSERT(size_.x > pos.x and size_.y > pos.y);
    }

public:
    [[nodiscard]] constexpr Vec2u size() const { return size_; }

    [[nodiscard]] constexpr const T * head_ptr() const {return this->resource_.get();}
    [[nodiscard]] constexpr T * head_ptr() {return this->resource_.get();}
    [[nodiscard]] constexpr std::shared_ptr<T[]> resource() const {return this->resource_;}
};



template<typename T>
__inline auto make_image(const Vec2u size){
    return Image<T>(size);
}

__inline auto make_gray_image(const Vec2u size){return make_image<Gray>(size);};
__inline auto make_bina_image(const Vec2u size){return make_image<Binary>(size);};


}

