#pragma once

// Wait-free SPSC 环形缓冲区，优化内存序
// - 单生产者单消费者无锁队列
// - 返回 size_t 表示操作数量（0表示失败）
// - 使用 consume_xxx() 实现零拷贝消费

#include <cstdint>
#include <span>
#include <memory>
#include <utility>
#include <atomic>

namespace ymd{

template<typename T, size_t N>
requires (std::has_single_bit(N))
class RingBuf final {
public:
    static constexpr size_t MASK = N - 1;
    static constexpr size_t MAX_CAPACITY = N - 1;  // 保留一个空位区分空/满
    
private:
    static constexpr size_t CACHE_LINE_SIZE = sizeof(size_t);

    alignas(CACHE_LINE_SIZE) std::byte storage_[N * sizeof(T)];
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> write_idx_{0};
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> read_idx_{0};
    
    T* data() noexcept {
        return std::launder(reinterpret_cast<T*>(storage_));
    }
    
    const T* data() const noexcept {
        return std::launder(reinterpret_cast<const T*>(storage_));
    }
    
    static constexpr size_t advance(size_t idx, size_t step = 1) noexcept {
        return (idx + step) & MASK;
    }
    
public:
    explicit RingBuf() noexcept = default;
    
    ~RingBuf() {
        clear();
    }
    
    // 禁止拷贝
    RingBuf(const RingBuf&) = delete;
    RingBuf& operator=(const RingBuf&) = delete;
    
    RingBuf(RingBuf&&) = delete;
    RingBuf& operator=(RingBuf&&) = delete;
    
    [[nodiscard]] static consteval size_t capacity() noexcept {
        return MAX_CAPACITY;  // 实际可用容量
    }
    
    // 单元素操作 - wait-free SPSC优化
    template<typename... Args>
    [[nodiscard]] size_t try_emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        const size_t write_idx = write_idx_.load(std::memory_order_relaxed);
        const size_t next_write = advance(write_idx);

        if (next_write == read_idx_.load(std::memory_order_acquire)) return 0;

        std::construct_at(data() + write_idx, std::forward<Args>(args)...);
        write_idx_.store(next_write, std::memory_order_release);
        return 1;
    }

    #if 0
    [[nodiscard]] size_t try_pop(T& value) noexcept(std::is_nothrow_move_assignable_v<T>) {
        const size_t read_idx = read_idx_.load(std::memory_order_relaxed);

        if (read_idx == write_idx_.load(std::memory_order_acquire)) return 0;

        value = std::move(data()[read_idx]);
        std::destroy_at(data() + read_idx);
        read_idx_.store(advance(read_idx), std::memory_order_release);
        return 1;
    }
    #endif
    

    template<typename Fn>
    [[nodiscard]] size_t consume_one(Fn&& fn)
        noexcept(noexcept(fn(std::declval<T&&>())))
    {
        const size_t read_idx = read_idx_.load(std::memory_order_relaxed);

        if (read_idx == write_idx_.load(std::memory_order_acquire)) return 0;

        std::forward<Fn>(fn)(std::move(data()[read_idx]));
        std::destroy_at(data() + read_idx);
        read_idx_.store(advance(read_idx), std::memory_order_release);
        return 1;
    }

    template<typename Fn>
    [[nodiscard]] size_t consume_each(Fn&& fn, size_t max_count = N)
        noexcept(noexcept(fn(std::declval<T&&>())))
    {
        const size_t read_idx = read_idx_.load(std::memory_order_relaxed);
        const size_t write_idx = write_idx_.load(std::memory_order_acquire);

        const size_t avail = (write_idx >= read_idx) ? (write_idx - read_idx) : (N - read_idx + write_idx);
        const size_t count = std::min(avail, max_count);
        if (count == 0) return 0;

        for (size_t i = 0; i < count; ++i) {
            const size_t idx = advance(read_idx, i);
            std::forward<Fn>(fn)(std::move(data()[idx]));
            std::destroy_at(data() + idx);
        }
        read_idx_.store(advance(read_idx, count), std::memory_order_release);
        return count;
    }

    [[nodiscard]] size_t try_push(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        const size_t write_idx = write_idx_.load(std::memory_order_relaxed);
        const size_t next_write = advance(write_idx);

        if (next_write == read_idx_.load(std::memory_order_acquire)) return 0;

        std::construct_at(data() + write_idx, value);
        write_idx_.store(next_write, std::memory_order_release);
        return 1;
    }

    [[nodiscard]] size_t try_push(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
        const size_t write_idx = write_idx_.load(std::memory_order_relaxed);
        const size_t next_write = advance(write_idx);

        if (next_write == read_idx_.load(std::memory_order_acquire)) return 0;

        std::construct_at(data() + write_idx, std::move(value));
        write_idx_.store(next_write, std::memory_order_release);
        return 1;
    }
    
    // 批量操作
    [[nodiscard]] size_t try_push(std::span<const T> src) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        const size_t write_idx = write_idx_.load(std::memory_order_relaxed);
        const size_t read_idx = read_idx_.load(std::memory_order_acquire);

        const size_t free_space = (read_idx > write_idx) ? (read_idx - write_idx - 1) : (N - write_idx + read_idx - 1);
        const size_t to_push = std::min(src.size(), free_space);

        if (to_push == 0) return 0;

        const size_t first_part = std::min(to_push, N - write_idx);
        const size_t second_part = to_push - first_part;

        for (size_t i = 0; i < first_part; ++i) {
            std::construct_at(data() + write_idx + i, src[i]);
        }

        for (size_t i = 0; i < second_part; ++i) {
            std::construct_at(data() + i, src[first_part + i]);
        }

        write_idx_.store(advance(write_idx, to_push), std::memory_order_release);
        return to_push;
    }
    
    [[nodiscard]] size_t try_pop(std::span<T> dst) noexcept(std::is_nothrow_move_assignable_v<T>) {
        const size_t read_idx = read_idx_.load(std::memory_order_relaxed);
        const size_t write_idx = write_idx_.load(std::memory_order_acquire);

        const size_t available = (write_idx >= read_idx) ? (write_idx - read_idx) : (N - read_idx + write_idx);
        const size_t to_pop = std::min(dst.size(), available);

        if (to_pop == 0) return 0;

        const size_t first_part = std::min(to_pop, N - read_idx);
        const size_t second_part = to_pop - first_part;

        for (size_t i = 0; i < first_part; ++i) {
            dst[i] = std::move(data()[read_idx + i]);
            std::destroy_at(data() + read_idx + i);
        }

        for (size_t i = 0; i < second_part; ++i) {
            dst[first_part + i] = std::move(data()[i]);
            std::destroy_at(data() + i);
        }

        read_idx_.store(advance(read_idx, to_pop), std::memory_order_release);
        return to_pop;
    }

    
    // 查询状态 - wait-free优化
    [[nodiscard]] size_t length() const noexcept {
        const size_t write_idx = write_idx_.load(std::memory_order_acquire);
        const size_t read_idx = read_idx_.load(std::memory_order_relaxed);
        return (write_idx >= read_idx) ? (write_idx - read_idx) : (N - read_idx + write_idx);
    }

    [[nodiscard]] bool is_empty() const noexcept {
        return read_idx_.load(std::memory_order_relaxed) == write_idx_.load(std::memory_order_acquire);
    }

    [[nodiscard]] bool is_full() const noexcept {
        const size_t write_idx = write_idx_.load(std::memory_order_relaxed);
        return advance(write_idx) == read_idx_.load(std::memory_order_acquire);
    }

    [[nodiscard]] size_t free_capacity() const noexcept {
        return MAX_CAPACITY - length();
    }
    
    // 管理操作
    void clear() noexcept {
        const size_t read_idx = read_idx_.load(std::memory_order_relaxed);
        const size_t write_idx = write_idx_.load(std::memory_order_acquire);

        const size_t len = (write_idx >= read_idx) ? (write_idx - read_idx) : (N - read_idx + write_idx);

        for (size_t i = 0; i < len; ++i) {
            std::destroy_at(data() + advance(read_idx, i));
        }

        read_idx_.store(0, std::memory_order_relaxed);
        write_idx_.store(0, std::memory_order_release);
    }
    
    [[nodiscard]] size_t waste(size_t len) noexcept {
        const size_t read_idx = read_idx_.load(std::memory_order_relaxed);
        const size_t write_idx = write_idx_.load(std::memory_order_acquire);

        const size_t available = (write_idx >= read_idx) ? (write_idx - read_idx) : (N - read_idx + write_idx);
        len = std::min(len, available);
        if (len == 0) return 0;

        for (size_t i = 0; i < len; ++i) {
            std::destroy_at(data() + advance(read_idx, i));
        }

        read_idx_.store(advance(read_idx, len), std::memory_order_release);
        return len;
    }
private:
};
}