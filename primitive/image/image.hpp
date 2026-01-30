#pragma once


#include <functional>
#include <memory>

#include "core/utils/Result.hpp"

#include "algebra/vectors/vec2.hpp"
#include "algebra/regions/rect2.hpp"
#include "primitive/colors/color/color.hpp"
#include "primitive/colors/rgb/rgb.hpp"



namespace ymd{

    
template <typename ColorType>
class Painter;

template <typename ColorType>
class PixelProxy;


enum class [[nodiscard]] ImageCreateError:uint8_t{
    OutOfMemory,
    EmptySize,
    LoadFromNull,
    RegionOutOfSize,
};


template<typename T>
class [[nodiscard]] Image final{
public:
    explicit Image(const math::Vec2u16 size):
        resource_(std::make_shared<T[]>(size.x * size.y)),
        size_(size){
            ASSERT(not size.is_zero());
        }

    explicit Image(std::shared_ptr<T[]> resource, const math::Vec2u16 size):
        resource_(resource),
        size_(size){
            ASSERT(not size.is_zero());
        }


    [[nodiscard]] static Image<T> clone_from_buf(const T * buf, const math::Vec2u16 size){
        auto resource = std::make_shared<T[]>(size.x * size.y);
        for(size_t i = 0; i < size.x * size.y; i++){
            resource.get()[i] = buf[i];
        }
        return Image<T>(std::move(resource), size);
    }

    Image(const Image & other) = delete;
    
    Image& operator=(Image&& other) noexcept {
        if (not this->is_shared_with(other)) {
            this->size_ = std::move(other.size_);
            this->resource_ = std::move(other.resource_);
            other.size_ = math::Vec2u16{0,0};
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

    [[nodiscard]] constexpr size_t position_to_index(const math::Vec2u16 pos) const {
        return pos.x + pos.y * this->size().x;
    }

    [[nodiscard]] Image<T> clone() const {
        const auto img_size = this->size();
        auto temp = Image<T>(img_size);

        for(size_t i = 0; i < img_size.x * img_size.y; i++){
            temp.head_ptr()[i] = this->head_ptr()[i];
        }
        return temp;
    }

    [[nodiscard]] Image<T> clone(const math::Rect2u16 & view) const {
        auto & self = *this;
        auto temp = Image<T>(view.size);
        for(uint16_t j = 0; j < view.h(); j++) {
            for(uint16_t i = 0; i < view.w(); i++) {
                temp[math::Vec2u16{i,j}] = self(i + view.x(), j + view.y());
            }
        }
        return temp;
    }

    [[nodiscard]] Image<T> copy(){
        return Image<T>((this->resource()),this->size());
    }

    template<typename ColorType2> 
    [[nodiscard]] Image<ColorType2> copy_as(){
        return Image<ColorType2>(
            std::reinterpret_pointer_cast<ColorType2[]>(this->resource()),
            this->size());
    }

    [[nodiscard]] constexpr bool is_shared_with(const Image<auto> & other) const {
        return resource_ == other.resource_;
    }

    void put_pixel(const math::Vec2u16 pos, const T color) {
        if(check_position_is_inrange(pos).is_err())
            return;
        put_pixel_unchecked(pos, color);
    }

    [[nodiscard]] T & operator[](const math::Vec2u16 pos) {
        return resource_[pos.x + pos.y * this->size().x]; }

    [[nodiscard]] const T & operator[](const math::Vec2u16 pos) const {
        return resource_[pos.x + pos.y * this->size().x]; }

    [[nodiscard]] T & operator()(const size_t x, const size_t y) {
        return resource_[x + y * this->size().x]; }

    [[nodiscard]] const T & operator()(const size_t x, const size_t y) const {
        return resource_[x + y * this->size().x]; }

    [[nodiscard]] T & at(const math::Vec2u16 pos){
        assert_position_is_inrange(pos);
        return resource_[pos.x + pos.y * this->size().x];
    }

    [[nodiscard]] const T & at(const math::Vec2u16 pos) const {
        assert_position_is_inrange(pos);
        return resource_[pos.x + pos.y * this->size().x];
    }

    void fill(const T color){
        const size_t area = this->size().x * this->size().y;
        for(size_t i = 0; i < area; i++){
            resource_[i] = color;
        }
    }

    void put_pixel_unchecked(const math::Vec2u16 pos, const T color) 
        { resource_[this->size().x * pos.y + pos.x] = color; }
    void get_pixel_unchecked(const math::Vec2u16 pos, T & color) const 
        { color = resource_[this->size().x * pos.y + pos.x]; }

    [[nodiscard]] constexpr size_t width() const { return size_.x; }
    [[nodiscard]] constexpr size_t height() const { return size_.y; }


private:
    std::shared_ptr<T[]> resource_;
    math::Vec2u16 size_ = math::Vec2u16::ZERO;

    void assert_position_is_inrange(const math::Vec2u16 pos){
        ASSERT(check_position_is_inrange(pos).is_ok());
    }

    [[nodiscard]] Result<void, void> check_position_is_inrange(const math::Vec2u16 pos){
        if((size_.x > pos.x) and (size_.y > pos.y)) return Ok();
        return Err();
    }

public:
    [[nodiscard]] constexpr math::Vec2u16 size() const { return size_; }

    [[nodiscard]] constexpr const T * head_ptr() const {return this->resource_.get();}
    [[nodiscard]] constexpr T * head_ptr() {return this->resource_.get();}
    [[nodiscard]] constexpr std::shared_ptr<T[]> resource() const {return this->resource_;}
};



template<typename T>
__inline auto make_image(const math::Vec2u16 size){
    return Image<T>(size);
}

__inline auto make_gray_image(const math::Vec2u16 size){return make_image<Gray>(size);};
__inline auto make_bina_image(const math::Vec2u16 size){return make_image<Binary>(size);};


}

