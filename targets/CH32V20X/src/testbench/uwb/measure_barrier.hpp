#pragma once

#include "core/utils/bits/atomic_bitset.hpp"

namespace ymd{

template<typename T, size_t N>
struct MeasurementBarrier {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for lock-free operations");
    
    using TimeStamp = float;
    
    // 设置测量值 - 返回是否成功设置新数据
    [[nodiscard]] __fast_inline bool set(size_t idx, T&& meas, float timestamp) {
        if (idx >= N) __builtin_abort();
        
        // 原子性地更新数据和时间戳
        buf_[idx] = std::forward<T>(meas);
        timestamps_[idx] = timestamp;
        
        // 内存屏障确保数据在设置标志位之前可见
        std::atomic_thread_fence(std::memory_order_release);
        
        // 设置标志位，如果之前未设置则返回true
        return flag_.test_and_set(idx);
    }
    
    // 检查是否所有传感器都有新数据
    [[nodiscard]] __fast_inline bool is_ready() const {
        return flag_.all();
    }
    
    // 检查时间同步性
    [[nodiscard]] __fast_inline bool is_timestamp_synced(float tolerance) const {
        if (!is_ready()) __builtin_abort();
        
        float min_ts = std::numeric_limits<float>::max();
        float max_ts = std::numeric_limits<float>::lowest();
        
        for (size_t i = 0; i < N; ++i) {
            float ts = buf_[i].timestamp;
            min_ts = ts < min_ts ? ts : min_ts;
            max_ts = ts > max_ts ? ts : max_ts;
        }
        
        return (max_ts - min_ts) <= tolerance;
    }
    
    // 获取所有数据的视图（不检查时间同步）
    [[nodiscard]] __fast_inline std::span<const T, N> get_all() const {
        if (!is_ready()) __builtin_abort();
        
        std::array<T, N> temp;
        for (size_t i = 0; i < N; ++i) {
            temp[i] = buf_[i];
        }
        
        // 注意：这里返回的是临时数组的span，调用方需要立即使用
        // 或者我们可以改变设计，让调用方提供输出数组
        static std::array<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = buf_[i];
        }
        return std::span<const T, N>(result);
    }
    
    // 获取同步的数据（检查时间同步）
    [[nodiscard]] __fast_inline std::span<const T, N> get_synced_data(float tolerance) {
        if (!is_ready()) __builtin_abort();
        if (!is_timestamp_synced(tolerance)) __builtin_abort();
        
        static std::array<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = buf_[i];
        }
        
        // 获取数据后清除标志位，准备下一轮
        flag_.clear_all();
        
        return std::span<const T, N>(result);
    }
    
    // 非阻塞尝试获取数据（带时间同步检查）
    [[nodiscard]] __fast_inline std::span<const T, N> try_get_synced(float tolerance) {
        if (!is_ready()) return {};
        if (!is_timestamp_synced(tolerance)) return {};
        
        static std::array<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = buf_[i];
        }
        
        flag_.clear_all();
        return std::span<const T, N>(result);
    }
    
    // 强制获取数据（忽略时间同步，用于紧急情况）
    [[nodiscard]] __fast_inline std::span<const T, N> force_get_all() {
        if (!is_ready()) __builtin_abort();
        
        static std::array<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = buf_[i];
        }
        
        flag_.clear_all();
        return std::span<const T, N>(result);
    }
    
    // 安全版本：将数据复制到提供的数组中
    __fast_inline void copy_all_to(std::array<T, N>& output) const {
        if (!is_ready()) __builtin_abort();
        
        for (size_t i = 0; i < N; ++i) {
            output[i] = buf_[i];
        }
    }
    
    __fast_inline void copy_synced_to(std::array<T, N>& output, float tolerance) {
        if (!is_ready()) __builtin_abort();
        if (!is_timestamp_synced(tolerance)) __builtin_abort();
        
        copy_all_to(output);
        flag_.clear_all();
    }
    
    [[nodiscard]] __fast_inline bool try_copy_synced_to(std::span<T, N> output, float tolerance) {
        if (!is_ready()) return false;
        if (!is_timestamp_synced(tolerance)) return false;
        
        copy_all_to(output);
        flag_.clear_all();
        return true;
    }
    // 获取当前数据状态信息（用于调试）
    // [[nodiscard]] __fast_inline size_t ready_count() const { return flag_.count(); }
    [[nodiscard]] __fast_inline float get_timestamp(size_t idx) const { 
        if (idx >= N) __builtin_abort();
        return timestamps_[idx]; 
    }
    
    // 手动重置屏障
    __fast_inline void reset() {
        flag_.clear_all();
    }

private:
    std::array<T, N> buf_;
    std::array<TimeStamp, N> timestamps_;
    AtomicBitset<N> flag_;
};

}