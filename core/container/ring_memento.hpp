#pragma once

#include <cstdint>
#include "core/utils/Option.hpp"

namespace ymd{


// 一个内部使用环形缓冲区，能不断插入新数据同时挤出末尾数据的容器
// 保障了右值安全
template<typename T, size_t N>
requires (std::has_single_bit(N))
struct [[nodiscard]] RingMemento {
    constexpr RingMemento() {
    }
    
    constexpr ~RingMemento() {
    }

    // 在首位插入数据
    constexpr void push_front(const T& item) {
        if (is_full()) {
            // 缓冲区已满，需要挤出末尾数据
            read_idx_ = (read_idx_ + 1) % N;
        }
        
        write_idx_ = (write_idx_ + N - 1) % N; // 向前移动
        data_[write_idx_] = item;
        
        if (!is_full() && size() == 0) {
            // 第一次插入时，read_idx_应该指向同一个位置
            read_idx_ = write_idx_;
        }
    }

    [[nodiscard]] constexpr size_t size() const {
        if (is_full()) {
            return N;
        }
        return (write_idx_ + N - read_idx_) % N;
    }

    [[nodiscard]] constexpr bool is_full() const {
        return (write_idx_ + 1) % N == read_idx_;
    }

    [[nodiscard]] constexpr bool is_empty() const {
        return write_idx_ == read_idx_;
    }

    [[nodiscard]] constexpr Option<T> at(const size_t idx) const {
        if (idx >= size()) {
            return None;
        }
        
        size_t actual_idx = (read_idx_ + idx) % N;
        return Some<T>(data_[actual_idx]); // 拷贝构造
    }

private:
    union {
        T data_[N];
    };
    size_t write_idx_ = 0;
    size_t read_idx_ = 0;

    // constexpr T pop_back()
};

}