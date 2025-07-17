#pragma once

#include "core/utils/Option.hpp"
#include <atomic>
#include <optional>
#include "core/sdk.hpp"
#include "core/utils/Result.hpp"

namespace ymd::core::sync{
namespace utils{

// Architecture-specific methods (same as before)
__fast_inline static void disable_interrupts() {
    __asm volatile ("csrw 0x800, %0" : : "r" (0x6000) );
}
__fast_inline static void enable_interrupts() {
    __asm volatile ("csrw 0x800, %0" : : "r" (0x6088) );
}
__fast_inline static void yield_cpu() {
    NVIC->SCTLR &= ~(1<<3);	// wfi
    __asm volatile ("wfi");
}

}

template<typename Fn>
__fast_inline static void spin_until(Fn && fn) {
    while(std::forward<Fn>(fn)() == false){
        __asm volatile ("nop");
    }
}

class CriticalSectionGuard {
public:
    CriticalSectionGuard() { utils::disable_interrupts(); }
    ~CriticalSectionGuard() { utils::enable_interrupts(); }
    // Copy/move operations deleted

    CriticalSectionGuard(const CriticalSectionGuard&) = delete;
    CriticalSectionGuard(CriticalSectionGuard&&) = delete;
};

class CountingSemaphore {
public:
    explicit CountingSemaphore(size_t initial = 0) : count_(initial) {}

    Result<void, void> acquire() {
        CriticalSectionGuard cs;
        if (count_ > 0) {
            --count_;
            return Ok();
        }
        return Err();
    }

    void release() {
        count_.fetch_add(1, std::memory_order_release);
    }

private:
    std::atomic<size_t> count_;
};


class BinarySemaphore {
public:
    Result<void, void> take() {
        CriticalSectionGuard cs;
        if (flag_) {
            flag_ = false;
            return Ok();
        }
        return Err();
    }

    void give() {
        flag_.store(true, std::memory_order_release);
    }

private:
    std::atomic<bool> flag_{false};
};

template<typename T>
class Mailbox {
public:
    Result<void, void> send(const T& msg) {
        CriticalSectionGuard cs;
        if (ctx_.is_none()) {
            ctx_ = Some(msg);
            return Ok();
        }
        return Err();
    }

    Option<T> receive() {
        CriticalSectionGuard cs;
        return ctx_;
    }

private:
    Option<T> ctx_;
};

class ReadWriteLock {
public:
    void read_lock() {
        while (true) {
            CriticalSectionGuard cs;
            if (writers_ == 0) {
                ++readers_;
                break;
            }
        }
    }

    void write_lock() {
        ++writers_;

        spin_until([this]{
            return readers_ <= 0;   
        });
    }

    void unlock() {
        if (writers_ > 0) {
            --writers_;
        } else {
            --readers_;
        }
    }

private:
    std::atomic<int> readers_{0};
    std::atomic<int> writers_{0};
};
}
