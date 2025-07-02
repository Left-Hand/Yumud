#pragma once

#include "image_basics.hpp"


namespace ymd{

template<typename T>
class Image final{
public:
    enum class Error{
        OutOfMemory,
        EmptySize,
        LoadFromNull,
        RegionOutOfSize,
    };

    template<typename U = void>
    using IResult = Result<U, Error>;

    Image(const Vector2u size):
        size_(size),
        data_(std::make_shared<T[]>(size.x * size.y)){
            ASSERT(bool(size));
        }

    // IResult<Image> from_size(const Vector2u size){
    //     auto raw = new (std::nothrow) T[size.x * size.y];
    //     if (!raw) {
    //         return Err(Error::OutOfMemory);
    //     }
    //     return Ok(std::move(Image(
    //         std::shared_ptr<T[]>(raw, std::default_delete<T[]>()),
    //         size))
    //     );
    // }

    Image(std::shared_ptr<T[]> data, const Vector2u size):
        size_(size),
        data_(data){
            ASSERT(bool(size));
        }


    [[nodiscard]] static Image<T> from_buf(const T * buf, const Vector2u size){
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
            this->data_ = std::move(other.data_);
            other.size_ = Vector2u{0,0};
            other.data_ = nullptr;
        }
        return *this;
    }
    
    Image(Image && other){
        *this = std::move(other);
    }
    Image & operator=(const Image& other) = delete;

    bool operator ==(const Image & other) const = delete;
    [[nodiscard]] Image<T> clone() const {
        const auto img_size = this->size();
        auto temp = Image<T>(img_size);

        for(size_t i = 0; i < img_size.x * img_size.y; i++){
            temp.get_data()[i] = this->get_data()[i];
        }
        return temp;
    }

    [[nodiscard]] Image<T> clone(const Rect2u & view) const {
        auto temp = Image<T>(view.size);
        for(size_t j = 0; j < view.h(); j++) {
            for(size_t i = 0; i < view.w(); i++) {
                temp[Vector2u{i,j}] = this->operator[](Vector2u{i + view.x(), j + view.y()});
            }
        }
        return temp;
    }

    template<typename ColorType2> 
    requires (sizeof(T) == sizeof(ColorType2))
    [[nodiscard]] Image<ColorType2> mirror(){
        return Image<ColorType2>(
            std::reinterpret_pointer_cast<T[]>(this->get_ptr()),
            this->size());
    }

    [[nodiscard]] constexpr bool is_shared_with(const Image<auto> & other) const {
        return data_ == other.data_;
    }

    void putpixel(const Vector2u pos, const T color) {
        assert_pos(pos);
        putpixel_unchecked(pos, color);
    }

    [[nodiscard]] __fast_inline const T & operator[](const size_t index) const { 
        return data_[index];
    }

    [[nodiscard]] __fast_inline T & operator[](const size_t index) {
        return data_[index]; }

    [[nodiscard]] __fast_inline T & operator[](const Vector2u pos) {
        return data_[pos.x + pos.y * this->size().x]; }

    [[nodiscard]] __fast_inline const T & operator[](const Vector2u pos) const {
        return data_[pos.x + pos.y * this->size().x]; }


    template<typename ToColorType>
    [[nodiscard]] __fast_inline ToColorType at(const Vector2u pos) const {
        assert_pos(pos);
        return data_[pos.x + pos.y * this->size().x]; }

    [[nodiscard]] __fast_inline T & at(const Vector2u pos){
        assert_pos(pos);
        return data_[pos.x + pos.y * this->size().x]; }

    [[nodiscard]] __fast_inline const T & at(const Vector2u pos)const{
        assert_pos(pos);
        return data_[pos.x + pos.y * this->size().x]; }

private:
    void putpixel_unchecked(const Vector2u pos, const T color) 
        { data_[this->size().x * pos.y + pos.x] = color; }
    void getpixel_unchecked(const Vector2u pos, T & color) const 
        { color = data_[this->size().x * pos.y + pos.x]; }


    Vector2u size_;

    std::shared_ptr<T[]> data_;

    __fast_inline void assert_pos(const Vector2u pos){
        ASSERT(size_.x > pos.x and size_.y > pos.y);
    }

public:
    constexpr Vector2u size() const { return size_; }

    constexpr auto get_data() const {return this->data_.get();}
    constexpr auto get_ptr() const {return this->data_;}
};



template<typename T>
__inline auto make_image(const Vector2u size){
    return Image<T>(size);
}

__inline auto make_gray_image(const Vector2u size){return make_image<Grayscale>(size);};
__inline auto make_bina_image(const Vector2u size){return make_image<Binary>(size);};


}

#include "image.tpp"

