#pragma once

#include <cstdint>
#include <memory>
#include "core/utils/Option.hpp"

namespace ymd{

// 环形备忘录容器 - 保留最新N条记录
// 应用场景：故障诊断需要获取最近的报文历史

template<typename T, size_t N>
requires (std::has_single_bit(N))
struct [[nodiscard]] RingMemento {
    static constexpr size_t MASK = N - 1;

    constexpr RingMemento() noexcept = default;

    constexpr ~RingMemento() {
        clear();
    }

    RingMemento(const RingMemento&) = delete;
    RingMemento& operator=(const RingMemento&) = delete;
    RingMemento(RingMemento&&) = delete;
    RingMemento& operator=(RingMemento&&) = delete;

    // 在首位插入数据
    constexpr void push_front(const T& item) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        write_idx_ = (write_idx_ - 1) & MASK;

        if (size_ == N) {
            data()[write_idx_] = item;
            read_idx_ = (read_idx_ - 1) & MASK;
        } else {
            std::construct_at(data() + write_idx_, item);
            ++size_;
        }
    }

    constexpr void push_front(T&& item) noexcept(std::is_nothrow_move_constructible_v<T>) {
        write_idx_ = (write_idx_ - 1) & MASK;

        if (size_ == N) {
            data()[write_idx_] = std::move(item);
            read_idx_ = (read_idx_ - 1) & MASK;
        } else {
            std::construct_at(data() + write_idx_, std::move(item));
            ++size_;
        }
    }

    template<typename... Args>
    constexpr void emplace_front(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        write_idx_ = (write_idx_ - 1) & MASK;

        if (size_ == N) {
            std::destroy_at(data() + write_idx_);
            std::construct_at(data() + write_idx_, std::forward<Args>(args)...);
            read_idx_ = (read_idx_ - 1) & MASK;
        } else {
            std::construct_at(data() + write_idx_, std::forward<Args>(args)...);
            ++size_;
        }
    }

    [[nodiscard]] constexpr size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
        return N;
    }

    [[nodiscard]] constexpr bool is_full() const noexcept {
        return size_ == N;
    }

    [[nodiscard]] constexpr bool is_empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] constexpr Option<T> at(size_t idx) const noexcept(std::is_nothrow_copy_constructible_v<T>) {
        if (idx >= size_) return None;
        return Some(data()[(write_idx_ + idx) & MASK]);
    }

    [[nodiscard]] constexpr const T& operator[](size_t idx) const noexcept {
        return data()[(write_idx_ + idx) & MASK];
    }

    constexpr void clear() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            std::destroy_at(data() + ((write_idx_ + i) & MASK));
        }
        size_ = 0;
        write_idx_ = 0;
        read_idx_ = 0;
    }

private:
    T* data() noexcept {
        return std::launder(reinterpret_cast<T*>(&storage_));
    }

    const T* data() const noexcept {
        return std::launder(reinterpret_cast<const T*>(&storage_));
    }

    alignas(T) std::byte storage_[N * sizeof(T)];
    size_t write_idx_ = 0;
    size_t read_idx_ = 0;
    size_t size_ = 0;
};

}