#pragma once

// #include "atomic.hpp"
#include <cstdint>
#include <atomic>
#include "utils.hpp"

#if 0
namespace ymd::core::sync{

class AtomicBarrier {
public:
    explicit AtomicBarrier(uint32_t count)
    : initial_count_(count),
        current_count_(count),
        generation_(0),
        release_flag_(false) {}

    void wait() {
        const uint32_t current_gen = generation_.load(std::memory_order_acquire);
        
        // Atomic decrement with interrupt protection
        utils::disable_interrupts();
        const uint32_t new_count = current_count_.fetch_sub(1, std::memory_order_relaxed) - 1;
        if(new_count == 0) {
            // Last thread arrived
            generation_.store(current_gen + 1, std::memory_order_release);
            current_count_.store(initial_count_.load(std::memory_order_relaxed), 
                std::memory_order_relaxed);
            release_flag_.store(true, std::memory_order_release);
        }
        utils::enable_interrupts();

        // Wait for barrier release
        while(true) {
            const bool released = release_flag_.load(std::memory_order_acquire);
            const uint32_t curr_gen = generation_.load(std::memory_order_acquire);
            
            if(released && curr_gen != current_gen) {
                release_flag_.store(false, std::memory_order_relaxed);
                break;
            }
            utils::yield_cpu();
        }
    }

    class BarrierGuard {
    public:
        explicit BarrierGuard(AtomicBarrier& barrier) 
            : barrier_(barrier) {
            barrier_.arrive_and_wait();
        }

        ~BarrierGuard() noexcept {
            // Optional cleanup logic if needed
        }

        BarrierGuard(const BarrierGuard&) = delete;
        BarrierGuard& operator=(const BarrierGuard&) = delete;
        
    private:
        AtomicBarrier& barrier_;
    };

    void arrive_and_wait() {
        wait();
    }

private:
    std::atomic<uint32_t> initial_count_;
    std::atomic<uint32_t> current_count_;
    std::atomic<uint32_t> generation_;
    std::atomic<bool> release_flag_;
};


}
#endif