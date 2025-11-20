#pragma once

#include <cstdint>
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "core/utils/result.hpp"
#include "core/utils/Option.hpp"

namespace ymd{

template<typename T, size_t N>
class [[nodiscard]] HeaplessVector {
public:
    // 构造函数 - constexpr
    constexpr HeaplessVector() noexcept : size_(0) {}

    static constexpr HeaplessVector from_empty(){
        return HeaplessVector<T, N>();
    } 

    template<size_t Extents>
    requires (Extents <= N || Extents == std::dynamic_extent)
    constexpr HeaplessVector(const std::span<const T, Extents> & elements) noexcept : 
        size_(elements.size()
    ){
        if constexpr(Extents == std::dynamic_extent)
            if(elements.size() > N) [[unlikely]]
                __builtin_trap();

        std::copy(elements.begin(), elements.end(), buf_);
    }

    template<typename ... Ts>
    requires (sizeof...(Ts) == N)
    constexpr HeaplessVector(Ts&& ... args) noexcept : 
        size_(sizeof...(Ts))
    {
        // Use an index sequence to properly forward each argument to the corresponding element
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ((buf_[Is] =  T(std::forward<Ts>(args))), ...);
        }(std::make_index_sequence<N>{});
    }


    constexpr HeaplessVector from_array(const std::array<T, N> & arr) noexcept{
        return HeaplessVector<T, N>(std::span<const T, N>(arr));
    }

    constexpr HeaplessVector from_list(const std::initializer_list<T> & list) noexcept{
        return HeaplessVector<T, N>(std::span<const T, N>(list));
    }

    // 析构函数 - constexpr
    constexpr ~HeaplessVector() {
        clear();
    }

    // 移动构造函数 - constexpr
    constexpr HeaplessVector(HeaplessVector&& other) noexcept {
        size_ = other.size_;
        for (size_t i = 0; i < size_; ++i) {
            new (&buf_[i]) T(std::move(other.buf_[i]));
            other.buf_[i].~T();
        }
        other.size_ = 0;
    }

    // 移动赋值 - constexpr
    constexpr HeaplessVector& operator=(HeaplessVector&& other) noexcept {
        if (this != &other) {
            clear();
            size_ = other.size_;
            for (size_t i = 0; i < size_; ++i) {
                new (&buf_[i]) T(std::move(other.buf_[i]));
                other.buf_[i].~T();
            }
            other.size_ = 0;
        }
        return *this;
    }

    // Safe copy constructor
    constexpr HeaplessVector(const HeaplessVector& other) {
        size_ = 0; // Initialize in case constructor throws
        for (size_t i = 0; i < other.size_; ++i) {
            new (&buf_[i]) T(other.buf_[i]); // Copy construct each element
            ++size_; // Only increment after successful construction
        }
    }

    // Safe copy assignment operator
    constexpr HeaplessVector& operator=(const HeaplessVector& other) {
        if (this != &other) {
            clear(); // Destroy existing elements
            size_ = 0;
            for (size_t i = 0; i < other.size_; ++i) {
                new (&buf_[i]) T(other.buf_[i]); // Copy construct each element
                ++size_;
            }
        }
        return *this;
    }

    // 元素访问 - constexpr
    [[nodiscard]] constexpr T& operator[](size_t index) {
        return buf_[index];
    }

    [[nodiscard]] constexpr const T& operator[](size_t index) const {
        return buf_[index];
    }

    [[nodiscard]] constexpr T& front() {
        return buf_[0];
    }

    [[nodiscard]] constexpr const T& front() const {
        return buf_[0];
    }

    [[nodiscard]] constexpr T& back() {
        return buf_[size_ - 1];
    }

    [[nodiscard]] constexpr const T& back() const {
        return buf_[size_ - 1];
    }

    [[nodiscard]] constexpr T * data() noexcept{
        return buf_;
    }
    [[nodiscard]] constexpr const T * data() const noexcept{
        return buf_;
    }


    // 容量相关 - constexpr
    [[nodiscard]] constexpr size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
        return N;
    }

    [[nodiscard]] constexpr bool is_empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] constexpr bool full() const noexcept {
        return size_ == N;
    }

    constexpr void push_back(const T& value) {
        if (size_ >= N) [[unlikely]]
            __builtin_trap(); // 或者 assert/throw/return error based on design
        new (&buf_[size_]) T(value);
        size_++;
    }

    constexpr void push_back(T&& value) {
        if (size_ >= N) [[unlikely]]
            __builtin_trap();
        new (&buf_[size_++]) T(std::move(value));
    }

    template<typename... Args>
    constexpr void emplace_back(Args&&... args) {
        if (size_ >= N) [[unlikely]]
            __builtin_trap();
        new (&buf_[size_++]) T(std::forward<Args>(args)...);
    }

    constexpr void append_unchecked(const T data){
        // ASSERT(size_ + 1 <= N);
        buf_[size_] = data;
        size_ = size_ + 1;
    }

    constexpr void append_unchecked(const std::span<const T> pbuf){
        // ASSERT(size_ + pbuf.size() <= N);
        for(size_t i = 0; i < pbuf.size(); i++){
            buf_[size_ + i] = pbuf[i];
        }
        size_ += pbuf.size();
    }

    [[nodiscard]] constexpr Result<void, void> append(const T data){
        if(size_ + 1 > N) return Err();
        buf_[size_] = data;
        size_ = size_ + 1;
        return Ok();
    }

    [[nodiscard]] constexpr Result<void, void> append(const std::span<const T> pbuf){
        if(size_ + pbuf.size() > N) return Err();
        for(size_t i = 0; i < pbuf.size(); i++){
            buf_[size_ + i] = pbuf[i];
        }
        size_ += pbuf.size();
        return Ok();
    }

    template<typename Iter>
    requires (is_std_iter_v<Iter>)
    [[nodiscard]] constexpr Result<void, void> append(Iter && iter){ 
        while(iter.has_next()){
            if(size_ + 1 > N) return Err();
            append_unchecked(iter.next());
        }
        return Ok();
    }

    [[nodiscard]] constexpr std::span<const T> as_slice() const {
        return std::span(buf_, size_);
    }

    constexpr void pop_back() {
        if(size_ == 0) [[unlikely]]
            __builtin_trap();
        buf_[--size_].~T();
    }

    constexpr void clear() {
        std::destroy_n(buf_, size_);
        size_ = 0;
    }

    // 迭代器 - constexpr
    [[nodiscard]] constexpr T* begin() noexcept {
        return buf_;
    }

    [[nodiscard]] constexpr const T* begin() const noexcept {
        return buf_;
    }

    [[nodiscard]] constexpr T* end() noexcept {
        return buf_ + size_;
    }

    [[nodiscard]] constexpr const T* end() const noexcept {
        return buf_ + size_;
    }

    [[nodiscard]] constexpr Option<const T &> at(size_t idx) const noexcept {
        if(idx > size_) return None;
        return Some(&buf_[idx]);
    }

    [[nodiscard]] constexpr T at_or(size_t idx, const T & other) const noexcept {
        if(idx > size_) return other;
        return buf_[idx];
    }

    [[nodiscard]] constexpr Option<T &> at(size_t idx) noexcept {
        if(idx > size_) return None;
        return Some(&buf_[idx]);
    }

    [[nodiscard]] constexpr std::span<const T> view() const {
        return std::span(buf_, size_);
    }

private:
    // 存储 - 使用 union 替代 aligned_storage (C++20 起支持 constexpr)
    union {
        T buf_[N];
    };
    size_t size_{0};
};

}