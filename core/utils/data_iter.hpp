#pragma once

#include <cstdint>
#include <span>

namespace ymd{


template<typename T>
struct [[nodiscard]] OnceIter{
    constexpr explicit OnceIter(const T value):
        value_(value){;}
    [[nodiscard]] __always_inline constexpr T next(){
        const auto ret = value_;
        is_done_ = true;
        return ret;
    }

    [[nodiscard]] __always_inline constexpr bool has_next() const{
        return is_done_ == false;
    }
private:
    T value_;
    bool is_done_ = false;
};

//ctad
template<typename T>
OnceIter(T) -> OnceIter<T>;

template<typename T, size_t Extents>
struct [[nodiscard]] RepeatIter{
    constexpr explicit RepeatIter(const T && value, size_t size = Extents):
        value_(std::move(value)){;}

    [[nodiscard]] __always_inline constexpr const T & next(){
        const auto & ret = value_;
        index_ ++;
        return ret;
    }

    [[nodiscard]] __always_inline constexpr bool has_next() const {
        return index_ < Extents;
    }
private:
    T value_;
    size_t index_ = 0;
};


template<typename T>
struct [[nodiscard]] RepeatIter<T, std::dynamic_extent>{
    constexpr explicit RepeatIter(const T value, size_t size):
        value_(value), size_(size){;}

    [[nodiscard]] __always_inline constexpr const T & next(){
        const auto ret = value_;
        index_ ++;
        return ret;
    }

    [[nodiscard]] __always_inline constexpr bool has_next() const {
        return index_ < size_;
    }
private:
    T value_;
    const size_t size_;
    size_t index_ = 0;
};


template<typename T>
struct [[nodiscard]] BurstIter{
    constexpr explicit BurstIter(const std::span<T> pbuf):
        pbuf_(pbuf){;}

    [[nodiscard]] __always_inline constexpr T & next(){
        auto & ret = pbuf_[index_];
        index_ ++;
        return ret;
    }
    
    [[nodiscard]] __always_inline constexpr bool has_next() const {
        return index_ < pbuf_.size();
    }
private:
    const std::span<T> pbuf_;
    size_t index_ = 0;
};

//ctad
template<typename T>
BurstIter(const std::span<T>) -> BurstIter<T>;
}