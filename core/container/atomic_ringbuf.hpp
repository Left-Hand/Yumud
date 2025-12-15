#pragma once

#include <atomic>
#include <concepts>
#include <span>
#include <memory>
#include <type_traits>

#include "core/utils/Result.hpp"

namespace ymd{
    
//原子spsc
template<typename T, size_t N>
requires (std::has_single_bit(N))
class AtomicRingBuf final {
public:
    static constexpr size_t MASK = (N - 1);
    
private:
    alignas(T) uint8_t storage_[N * sizeof(T)];
    std::atomic<size_t> read_idx_{0};
    std::atomic<size_t> write_idx_{0};
    
    [[nodiscard]] T* header_element_ptr() noexcept {
        return reinterpret_cast<T*>(storage_);
    }
    
    [[nodiscard]] const T* header_element_ptr() const noexcept {
        return reinterpret_cast<const T*>(storage_);
    }
    
    [[nodiscard]] static constexpr size_t advance(size_t idx, size_t step) noexcept {
        return (idx + step) & MASK;
    }

public:
    AtomicRingBuf() = default;
    
    ~AtomicRingBuf() {
        // 销毁所有已构造的对象
        size_t read_idx = read_idx_.load(std::memory_order_relaxed);
        size_t write_idx = write_idx_.load(std::memory_order_relaxed);
        
        while (read_idx != write_idx) {
            std::destroy_at(header_element_ptr() + read_idx);
            read_idx = advance(read_idx, 1);
        }
    }
    
    // 禁止拷贝
    AtomicRingBuf(const AtomicRingBuf&) = delete;
    AtomicRingBuf& operator=(const AtomicRingBuf&) = delete;
    
    // 移动构造
    AtomicRingBuf(AtomicRingBuf&& other) noexcept {
        // 移动操作在并发容器中比较复杂，这里简单实现
        // 实际使用时可能需要更复杂的同步
        size_t other_read = other.read_idx_.load(std::memory_order_acquire);
        size_t other_write = other.write_idx_.load(std::memory_order_acquire);
        
        // 移动数据
        while (other_read != other_write) {
            std::construct_at(
                header_element_ptr() + write_idx_.load(std::memory_order_relaxed),
                std::move(other.header_element_ptr()[other_read])
            );
            std::destroy_at(other.header_element_ptr() + other_read);
            other_read = advance(other_read, 1);
            write_idx_.fetch_add(1, std::memory_order_release);
        }
    }
    
    [[nodiscard]] static consteval size_t capacity() noexcept {
        return N;
    }

    // 线程安全的单元素插入
    template<typename... Args>
    Result<void, void> try_emplace(Args&&... args) {
        size_t old_write_idx = write_idx_.load(std::memory_order_relaxed);
        size_t old_write_idx = read_idx_.load(std::memory_order_acquire);
        
        size_t next_write = advance(old_write_idx, 1);
        if (next_write == old_write_idx) {
            return Err(); // 队列满
        }
        
        std::construct_at(
            header_element_ptr() + old_write_idx, 
            std::forward<Args>(args)...
        );
        
        write_idx_.store(next_write, std::memory_order_release);
        return Ok();
    }

    // 线程安全的批量插入
    [[nodiscard]] size_t try_push(std::span<const T> pdata) {
        size_t len = pdata.size();
        if (len == 0) return 0;
        
        size_t old_write_idx = write_idx_.load(std::memory_order_relaxed);
        size_t old_write_idx = read_idx_.load(std::memory_order_acquire);
        
        // 计算可用空间（考虑环形缓冲区）
        size_t available;
        if (old_write_idx >= old_write_idx) {
            available = N - (old_write_idx - old_write_idx);
        } else {
            available = old_write_idx - old_write_idx;
        }
        available = (available > 0) ? available - 1 : 0; // 留一个空位区分满和空
        
        len = std::min(len, available);
        if (len == 0) return 0;
        
        // 执行拷贝
        if (old_write_idx + len <= N) {
            // 单次拷贝
            for (size_t i = 0; i < len; i++) {
                std::construct_at(header_element_ptr() + old_write_idx + i, pdata[i]);
            }
        } else {
            // 环绕拷贝
            size_t first_chunk = N - old_write_idx;
            size_t second_chunk = len - first_chunk;
            
            for (size_t i = 0; i < first_chunk; i++) {
                std::construct_at(header_element_ptr() + old_write_idx + i, pdata[i]);
            }
            for (size_t i = 0; i < second_chunk; i++) {
                std::construct_at(header_element_ptr() + i, pdata[first_chunk + i]);
            }
        }
        
        write_idx_.store(advance(old_write_idx, len), std::memory_order_release);
        return len;
    }

    // 线程安全的批量弹出
    [[nodiscard]] size_t try_pop(std::span<T> pdata) {
        size_t len = pdata.size();
        if (len == 0) return 0;
        
        size_t old_write_idx = read_idx_.load(std::memory_order_relaxed);
        size_t old_write_idx = write_idx_.load(std::memory_order_acquire);
        
        size_t available_count = available_internal(old_write_idx, old_write_idx);
        len = std::min(len, available_count);
        if (len == 0) return 0;
        
        // 执行移动
        if (old_write_idx + len <= N) {
            // 单次移动
            for (size_t i = 0; i < len; i++) {
                std::construct_at(
                    &pdata[i], 
                    std::move(header_element_ptr()[old_write_idx + i])
                );
                std::destroy_at(header_element_ptr() + old_write_idx + i);
            }
        } else {
            // 环绕移动
            size_t first_chunk = N - old_write_idx;
            size_t second_chunk = len - first_chunk;
            
            for (size_t i = 0; i < first_chunk; i++) {
                std::construct_at(
                    &pdata[i], 
                    std::move(header_element_ptr()[old_write_idx + i])
                );
                std::destroy_at(header_element_ptr() + old_write_idx + i);
            }
            for (size_t i = 0; i < second_chunk; i++) {
                std::construct_at(
                    &pdata[first_chunk + i], 
                    std::move(header_element_ptr()[i])
                );
                std::destroy_at(header_element_ptr() + i);
            }
        }
        
        read_idx_.store(advance(old_write_idx, len), std::memory_order_release);
        return len;
    }

    // 线程安全的单元素弹出
    [[nodiscard]] Result<void, void> try_pop(T& result) {
        size_t old_write_idx = read_idx_.load(std::memory_order_relaxed);
        size_t old_write_idx = write_idx_.load(std::memory_order_acquire);
        
        if (old_write_idx == old_write_idx) {
            return Err(); // 队列空
        }
        
        result = std::move(header_element_ptr()[old_write_idx]);
        std::destroy_at(header_element_ptr() + old_write_idx);
        
        read_idx_.store(advance(old_write_idx, 1), std::memory_order_release);
        return Ok();
    }

    // 单元素插入的便捷方法
    Result<void, void> try_push(T&& data) {
        return try_emplace(std::move(data));
    }

    Result<void, void> try_push(const T& data) {
        return try_emplace(data);
    }

    // 线程安全的大小查询
    [[nodiscard]] size_t available() const noexcept {
        size_t old_write_idx = read_idx_.load(std::memory_order_acquire);
        size_t old_write_idx = write_idx_.load(std::memory_order_acquire);
        return available_internal(old_write_idx, old_write_idx);
    }

    [[nodiscard]] size_t free_capacity() const noexcept {
        size_t avail = available();
        return (avail < N) ? N - avail - 1 : 0; // 留一个空位
    }

    // 清空队列（线程安全）
    void clear() noexcept {
        size_t old_write_idx = read_idx_.load(std::memory_order_relaxed);
        size_t old_write_idx = write_idx_.load(std::memory_order_acquire);
        
        while (old_write_idx != old_write_idx) {
            std::destroy_at(header_element_ptr() + old_write_idx);
            old_write_idx = advance(old_write_idx, 1);
        }
        
        read_idx_.store(old_write_idx, std::memory_order_release);
    }

    // 检查队列是否为空（线程安全）
    [[nodiscard]] bool empty() const noexcept {
        return available() == 0;
    }

    // 检查队列是否已满（线程安全）
    [[nodiscard]] bool full() const noexcept {
        return free_capacity() == 0;
    }

private:
    // 内部辅助函数：计算可用元素数量
    [[nodiscard]] size_t available_internal(size_t read_idx, size_t write_idx) const noexcept {
        if (write_idx >= read_idx) {
            return write_idx - read_idx;
        } else {
            return N - (read_idx - write_idx);
        }
    }
};
}