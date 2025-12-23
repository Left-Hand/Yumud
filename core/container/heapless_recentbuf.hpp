#pragma once

#include <cstdint>
#include "core/utils/Option.hpp"

namespace ymd::heapless{


// 主类 RecentBuf
template<typename T, size_t N>
requires (std::has_single_bit(N))
struct [[nodiscard]] RecentBuf {
    static constexpr size_t MASK = N - 1;   
    
    constexpr RecentBuf() {
        // 使用 placement new 初始化数组
        for (size_t i = 0; i < N; ++i) {
            new (&resource_[i]) T();
        }
        write_idx_ = 0;
        read_idx_ = 0;
        size_ = 0;
    }
    
    // 拷贝构造函数
    constexpr RecentBuf(const RecentBuf& other) {
        for (size_t i = 0; i < N; ++i) {
            new (&resource_[i]) T(other.resource_[i]);
        }
        write_idx_ = other.write_idx_;
        read_idx_ = other.read_idx_;
        size_ = other.size_;
    }
    
    // 移动构造函数
    constexpr RecentBuf(RecentBuf&& other) noexcept {
        for (size_t i = 0; i < N; ++i) {
            new (&resource_[i]) T(std::move(other.resource_[i]));
        }
        write_idx_ = other.write_idx_;
        read_idx_ = other.read_idx_;
        size_ = other.size_;
        
        // 清空原对象
        other.write_idx_ = 0;
        other.read_idx_ = 0;
        other.size_ = 0;
    }
    
    constexpr ~RecentBuf() {
        // 析构所有元素
        for (size_t i = 0; i < N; ++i) {
            resource_[i].~T();
        }
    }
    
    // 赋值运算符
    constexpr RecentBuf& operator=(const RecentBuf& other) {
        if (this != &other) {
            // 析构现有元素
            for (size_t i = 0; i < N; ++i) {
                resource_[i].~T();
            }
            
            // 拷贝新元素
            for (size_t i = 0; i < N; ++i) {
                new (&resource_[i]) T(other.resource_[i]);
            }
            
            write_idx_ = other.write_idx_;
            read_idx_ = other.read_idx_;
            size_ = other.size_;
        }
        return *this;
    }
    
    constexpr RecentBuf& operator=(RecentBuf&& other) noexcept {
        if (this != &other) {
            // 析构现有元素
            for (size_t i = 0; i < N; ++i) {
                resource_[i].~T();
            }
            
            // 移动新元素
            for (size_t i = 0; i < N; ++i) {
                new (&resource_[i]) T(std::move(other.resource_[i]));
            }
            
            write_idx_ = other.write_idx_;
            read_idx_ = other.read_idx_;
            size_ = other.size_;
            
            // 清空原对象
            other.write_idx_ = 0;
            other.read_idx_ = 0;
            other.size_ = 0;
        }
        return *this;
    }
    
    // 在首位插入数据（最新位置），如果已满，挤出末尾的数据（最旧位置）
    constexpr void push(const T& item) {
        if (is_full()) {
            // 缓冲区已满，移除最旧元素
            read_idx_ = (read_idx_ + 1) & MASK;
            size_ = N; // 保持大小不变
        } else {
            size_++;
        }
        
        // write_idx_ 指向最新的元素位置
        write_idx_ = (write_idx_ + N - 1) & MASK; // 向前移动
        resource_[write_idx_] = item;
        
        // 如果是第一个元素，初始化 read_idx_
        if (size_ == 1) {
            read_idx_ = write_idx_;
        }
    }
    
    constexpr void push(T&& item) {
        if (is_full()) {
            // 缓冲区已满，移除最旧元素
            read_idx_ = (read_idx_ + 1) & MASK;
            size_ = N; // 保持大小不变
        } else {
            size_++;
        }
        
        // write_idx_ 指向最新的元素位置
        write_idx_ = (write_idx_ + N - 1) & MASK; // 向前移动
        resource_[write_idx_] = std::move(item);
        
        // 如果是第一个元素，初始化 read_idx_
        if (size_ == 1) {
            read_idx_ = write_idx_;
        }
    }
    
    // 尝试弹出最新的元素（后进先出）
    constexpr Option<T> try_pop() {
        if (is_empty()) {
            return None;
        }
        
        // 获取最新的元素
        Option<T> result = Some<T>(std::move(resource_[write_idx_]));
        
        // 移动 write_idx_ 到下一个最新位置
        write_idx_ = (write_idx_ + 1) & MASK;
        size_--;
        
        // 如果缓冲区变空，重置 read_idx_
        if (is_empty()) {
            read_idx_ = write_idx_;
        }
        
        return result;
    }
    
    // 查看最新的元素但不移除
    [[nodiscard]] constexpr Option<const T&> peek() const {
        if (is_empty()) {
            return None;
        }
        return Some<const T&>(resource_[write_idx_]);
    }
    
    [[nodiscard]] constexpr Option<T&> peek_mut() {
        if (is_empty()) {
            return None;
        }
        return Some<T&>(resource_[write_idx_]);
    }
    
    // 查看最旧的元素但不移除
    [[nodiscard]] constexpr Option<const T&> peek_oldest() const {
        if (is_empty()) {
            return None;
        }
        return Some<const T&>(resource_[read_idx_]);
    }
    
    // 获取容量
    [[nodiscard]] static constexpr size_t capacity() noexcept {
        return N;
    }
    
    // 获取当前大小
    [[nodiscard]] constexpr size_t size() const noexcept {
        return size_;
    }
    
    // 判断是否已满
    [[nodiscard]] constexpr bool is_full() const noexcept {
        return size_ == N;
    }
    
    // 判断是否为空
    [[nodiscard]] constexpr bool is_empty() const noexcept {
        return size_ == 0;
    }
    
    // 访问元素，索引 0 表示最新的元素，索引 size()-1 表示最旧的元素
    [[nodiscard]] constexpr const T& operator[](size_t idx) const {
        // idx = 0 表示最新的元素（write_idx_）
        // idx = size()-1 表示最旧的元素（read_idx_）
        size_t actual_idx = (write_idx_ + idx) & MASK;
        return resource_[actual_idx];
    }
    
    [[nodiscard]] constexpr T& operator[](size_t idx) {
        size_t actual_idx = (write_idx_ + idx) & MASK;
        return resource_[actual_idx];
    }
    
    // 安全的元素访问，返回 Option
    [[nodiscard]] constexpr Option<const T&> at(size_t idx) const {
        if (idx >= size_) {
            return None;
        }
        return Some<const T&>((*this)[idx]);
    }
    
    [[nodiscard]] constexpr Option<T&> at(size_t idx) {
        if (idx >= size_) {
            return None;
        }
        return Some<T&>((*this)[idx]);
    }
    
    // 清空缓冲区
    constexpr void clear() noexcept {
        // 不需要析构元素，因为它们仍然存在，只是标记为空
        size_ = 0;
        write_idx_ = 0;
        read_idx_ = 0;
    }
    
    // 转换为数组（按最新到最旧的顺序）
    template<size_t M = N>
    [[nodiscard]] constexpr std::array<T, M> to_array() const {
        std::array<T, M> result{};
        const size_t copy_size = size_ < M ? size_ : M;
        
        for (size_t i = 0; i < copy_size; ++i) {
            result[i] = (*this)[i];
        }
        
        return result;
    }
    
    // 迭代器支持
    class Iterator {
    private:
        const RecentBuf* buf_;
        size_t index_;
        
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
        
        constexpr Iterator(const RecentBuf* buf, size_t index)
            : buf_(buf), index_(index) {}
        
        constexpr reference operator*() const {
            return (*buf_)[index_];
        }
        
        constexpr pointer operator->() const {
            return &(*buf_)[index_];
        }
        
        constexpr Iterator& operator++() {
            ++index_;
            return *this;
        }
        
        constexpr Iterator operator++(int) {
            Iterator temp = *this;
            ++index_;
            return temp;
        }
        
        constexpr bool operator==(const Iterator& other) const {
            return buf_ == other.buf_ && index_ == other.index_;
        }
        
        constexpr bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };
    
    [[nodiscard]] constexpr Iterator begin() const {
        return Iterator(this, 0);
    }
    
    [[nodiscard]] constexpr Iterator end() const {
        return Iterator(this, size_);
    }
    
    // 反向迭代器（从最旧到最新）
    class ReverseIterator {
    private:
        const RecentBuf* buf_;
        size_t index_;
        
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
        
        constexpr ReverseIterator(const RecentBuf* buf, size_t index)
            : buf_(buf), index_(index) {}
        
        constexpr reference operator*() const {
            return (*buf_)[index_];
        }
        
        constexpr pointer operator->() const {
            return &(*buf_)[index_];
        }
        
        constexpr ReverseIterator& operator++() {
            --index_;
            return *this;
        }
        
        constexpr ReverseIterator operator++(int) {
            ReverseIterator temp = *this;
            --index_;
            return temp;
        }
        
        constexpr bool operator==(const ReverseIterator& other) const {
            return buf_ == other.buf_ && index_ == other.index_;
        }
        
        constexpr bool operator!=(const ReverseIterator& other) const {
            return !(*this == other);
        }
    };
    
    [[nodiscard]] constexpr ReverseIterator rbegin() const {
        return ReverseIterator(this, size_ - 1);
    }
    
    [[nodiscard]] constexpr ReverseIterator rend() const {
        return ReverseIterator(this, -1); // 使用 -1 作为结束标记
    }

private:
    union {
        T resource_[N];
    };
    size_t write_idx_ = 0;    // 指向最新的元素
    size_t read_idx_ = 0;     // 指向最旧的元素
    size_t size_ = 0;         // 当前元素数量
};

}

namespace ymd{
template<typename T, size_t N>
using HeaplessRecentBuf = heapless::RecentBuf<T, N>;
}