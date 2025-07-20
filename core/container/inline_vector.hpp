#pragma once

#include <cstdint>
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ymd{

template<typename T, size_t N>
class InlineVector {
public:
    // 构造函数 - constexpr
    constexpr InlineVector() noexcept : size_(0) {}

    // 析构函数 - constexpr
    constexpr ~InlineVector() {
        clear();
    }

    // 移动构造函数 - constexpr
    constexpr InlineVector(InlineVector&& other) noexcept {
        size_ = other.size_;
        for (size_t i = 0; i < size_; ++i) {
            new (&data_[i]) T(std::move(other.data_[i]));
            other.data_[i].~T();
        }
        other.size_ = 0;
    }

    // 移动赋值 - constexpr
    constexpr InlineVector& operator=(InlineVector&& other) noexcept {
        if (this != &other) {
            clear();
            size_ = other.size_;
            for (size_t i = 0; i < size_; ++i) {
                new (&data_[i]) T(std::move(other.data_[i]));
                other.data_[i].~T();
            }
            other.size_ = 0;
        }
        return *this;
    }

    // Safe copy constructor
    constexpr InlineVector(const InlineVector& other) {
        size_ = 0; // Initialize in case constructor throws
        for (size_t i = 0; i < other.size_; ++i) {
            new (&data_[i]) T(other.data_[i]); // Copy construct each element
            ++size_; // Only increment after successful construction
        }
    }

    // Safe copy assignment operator
    constexpr InlineVector& operator=(const InlineVector& other) {
        if (this != &other) {
            clear(); // Destroy existing elements
            size_ = 0;
            for (size_t i = 0; i < other.size_; ++i) {
                new (&data_[i]) T(other.data_[i]); // Copy construct each element
                ++size_;
            }
        }
        return *this;
    }

    // 元素访问 - constexpr
    constexpr T& operator[](size_t index) {
        return data_[index];
    }

    constexpr const T& operator[](size_t index) const {
        return data_[index];
    }

    constexpr T& front() {
        return data_[0];
    }

    constexpr const T& front() const {
        return data_[0];
    }

    constexpr T& back() {
        return data_[size_ - 1];
    }

    constexpr const T& back() const {
        return data_[size_ - 1];
    }

    constexpr T * data() noexcept{
        return data_;
    }
    constexpr const T * data() const noexcept{
        return data_;
    }


    // 容量相关 - constexpr
    constexpr size_t size() const noexcept {
        return size_;
    }

    constexpr size_t capacity() const noexcept {
        return N;
    }

    constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    constexpr bool full() const noexcept {
        return size_ == N;
    }

    // 修改操作 - constexpr
    constexpr void push_back(const T& value) {
        new (&data_[size_++]) T(value);
    }

    constexpr void push_back(T&& value) {
        new (&data_[size_++]) T(std::move(value));
    }

    template<typename... Args>
    constexpr void emplace_back(Args&&... args) {
        new (&data_[size_++]) T(std::forward<Args>(args)...);
    }

    constexpr void append_unchecked(const uint8_t data){
        // ASSERT(size_ + 1 <= N);
        data_[size_] = data;
        size_ = size_ + 1;
    }

    constexpr void append_unchecked(const std::span<const uint8_t> pbuf){
        // ASSERT(size_ + pbuf.size() <= N);
        for(size_t i = 0; i < pbuf.size(); i++){
            data_[size_ + i] = pbuf[i];
        }
        size_ += pbuf.size();
    }

    constexpr std::span<const uint8_t> iter() const {
        return std::span(data_, size_);
    }

    constexpr void pop_back() {
        data_[--size_].~T();
    }

    constexpr void clear() {
        while (size_ > 0) {
            pop_back();
        }
    }

    // 迭代器 - constexpr
    constexpr T* begin() noexcept {
        return data_;
    }

    constexpr const T* begin() const noexcept {
        return data_;
    }

    constexpr T* end() noexcept {
        return data_ + size_;
    }

    constexpr const T* end() const noexcept {
        return data_ + size_;
    }

private:
    // 存储 - 使用 union 替代 aligned_storage (C++20 起支持 constexpr)
    union {
        T data_[N];
    };
    size_t size_{0};
};

}