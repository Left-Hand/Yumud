#pragma once

#include "core/system.hpp"
#include <atomic>

namespace ymd::hal{

class PeripheralOwnershipTracker final {
private:
    // 使用单个原子变量打包所有状态，避免位域的非原子性问题
    struct State {
        uint32_t req_id : 16;
        uint32_t on_interrupt : 1;
        uint32_t is_borrowed : 1;
    };
    
    static_assert(sizeof(State) == sizeof(uint32_t), "State should be 32 bits");
    
    std::atomic<uint32_t> state_{0};

    // 从原子值解码状态
    __fast_inline State load_state() const noexcept {
        return std::bit_cast<State>(state_.load(std::memory_order_acquire));
    }


public:
    __fast_inline PeripheralOwnershipTracker() = default;

    __fast_inline ~PeripheralOwnershipTracker() {
        lend();
    }

    PeripheralOwnershipTracker(const PeripheralOwnershipTracker& other) = delete;
    PeripheralOwnershipTracker(PeripheralOwnershipTracker&& other) = delete;
    PeripheralOwnershipTracker& operator=(const PeripheralOwnershipTracker& other) = delete;
    PeripheralOwnershipTracker& operator=(PeripheralOwnershipTracker&& other) = delete;

    template<typename BorrowRequest>
    void borrow(const BorrowRequest req) {
        // 禁用中断以确保操作的原子性
        sys::exception::disable_interrupt();
        
        State new_state{
            .req_id = req.as_unique_id(),
            .on_interrupt = sys::exception::is_interrupt_acting(),
            .is_borrowed = true
        };
        
        // 原子性地更新所有状态
        state_.store(std::bit_cast<uint32_t>(new_state), std::memory_order_release);
        
        sys::exception::enable_interrupt();
    }

    __fast_inline void lend() noexcept {
        // 使用原子交换操作确保线程安全
        State current = load_state();
        if (current.is_borrowed) {
            current.is_borrowed = false;
            state_.store(std::bit_cast<uint32_t>(current), std::memory_order_release);
        }
    }

    template<typename BorrowRequest>
    bool is_borrowed_by(const BorrowRequest req) const noexcept {
        // 一次性读取所有状态，确保一致性
        State current = load_state();
        return (current.is_borrowed && 
                current.req_id == req.as_unique_id() &&
                current.on_interrupt == sys::exception::is_interrupt_acting());
    }

    [[nodiscard]] __fast_inline bool is_borrowed() const noexcept {
        return load_state().is_borrowed;
    }

    // 可选：提供更细粒度的状态查询
    [[nodiscard]] __fast_inline uint32_t requester_id() const noexcept {
        return load_state().req_id;
    }

    [[nodiscard]] __fast_inline bool was_borrowed_in_interrupt() const noexcept {
        return load_state().on_interrupt;
    }

    #if 0
    // 原子性比较和交换操作，用于高级用例
    template<typename BorrowRequest>
    bool try_transfer_ownership(const BorrowRequest from, const BorrowRequest to) {
        sys::exception::disable_interrupt();
        
        State expected = load_state();
        State desired = expected;
        
        bool success = false;
        if (expected.is_borrowed && 
            expected.req_id == from.as_unique_id() &&
            expected.on_interrupt == sys::exception::is_interrupt_acting()) {
            
            desired.req_id = to.as_unique_id();
            success = state_.compare_exchange_strong(
                reinterpret_cast<uint32_t &>(expected), 
                std::bit_cast<uint32_t>(desired),
                std::memory_order_acq_rel,
                std::memory_order_acquire
            );
        }
        
        sys::exception::enable_interrupt();
        return success;
    }
    #endif
};

};
