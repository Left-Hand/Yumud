#pragma once

#include <cstdint>
#include <span>

namespace ymd{


template<typename T>
struct OnceIter{
    constexpr explicit OnceIter(const T value):
        value_(value){;}
    constexpr T next(){
        const auto ret = value_;
        is_done_ = true;
        return ret;
    }

    constexpr bool has_next() const{
        return is_done_ == false;
    }
private:
    T value_;
    bool is_done_ = false;
};

//ctad
template<typename T>
OnceIter(T) -> OnceIter<T>;

template<typename T>
struct RepeatIter{
    constexpr explicit RepeatIter(const T value, size_t size):
        value_(value), size_(size){;}

    constexpr T next(){
        const auto ret = value_;
        index_ ++;
        return ret;
    }
    constexpr bool has_next() const {
        return index_ < size_;
    }
private:
    T value_;
    const size_t size_;
    size_t index_ = 0;
};

//ctad
template<typename T>
RepeatIter(T, size_t) -> RepeatIter<T>;

template<typename T>
struct BurstIter{
    constexpr explicit BurstIter(const std::span<const T> pbuf):
        pbuf_(pbuf){;}

    constexpr T next(){
        const auto ret = pbuf_[index_];
        index_ ++;
        return ret;
    }
    
    constexpr bool has_next() const {
        return index_ + 1 >= pbuf_.size();
    }
private:
    const std::span<const T> pbuf_;
    size_t index_ = 0;
};

//ctad
template<typename T>
BurstIter(const std::span<const T>) -> BurstIter<T>;
}