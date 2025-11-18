#pragma once

#include <atomic>
#include <cstddef>
#include "core/utils/Result.hpp"

namespace ymd{

template<size_t N>
struct [[nodiscard]] AtomicBitset {
    static_assert(N <= 64, "Only support up to 64 bits for atomic operations");
    
    [[nodiscard]] Result<void, void> test_and_set(size_t idx) {
        uint64_t mask = 1ULL << idx;
        uint64_t old = bits_.fetch_or(mask, std::memory_order_acq_rel);
        if((old & mask) != 0) return Err(); // 返回true表示之前未设置
        return Ok();
    }
    
    void set(size_t idx) {
        bits_.fetch_or(1ULL << idx, std::memory_order_release);
    }
    
    [[nodiscard]]  bool test(size_t idx) const {
        return (bits_.load(std::memory_order_acquire) & (1ULL << idx)) != 0;
    }
    
    void unset(size_t idx) {
        bits_.fetch_and(~(1ULL << idx), std::memory_order_release);
    }
    
    void clear() {
        bits_.store(0, std::memory_order_release);
    }
    
    [[nodiscard]] bool all() const {
        return bits_.load(std::memory_order_acquire) == ((1ULL << N) - 1);
    }
    
    [[nodiscard]] size_t count() const {
        return __builtin_popcountll(bits_.load(std::memory_order_acquire));
    }
    
private:
    std::atomic<uint64_t> bits_{0};
};
}