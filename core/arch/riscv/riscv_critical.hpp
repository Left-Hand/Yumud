#pragma once

#ifdef __riscv
#include <cstdint>

namespace ymd::cpu_core{

static constexpr uint32_t MIE_BITMASK = (1u << 3);
static inline void enable_irq()
{
    __asm volatile ("csrw 0x800, %0" : : "r" (0x6088) );
}

static inline void disable_irq()
{
    __asm volatile ("csrw 0x800, %0" : : "r" (0x6000) );
}


[[nodiscard]] static inline uint32_t get_MSTATUS(void){
    uint32_t result;

    __asm volatile ( "csrr %0," "mstatus" : "=r" (result) );
    return (result);
}

static inline void set_MSTATUS(uint32_t value)
{
    __asm volatile ("csrw mstatus, %0" : : "r" (value) );
}

static inline bool get_PRIMASK(void) {
    uint32_t mstatus = get_MSTATUS();
    return (mstatus & (1 << 3)) ? 1 : 0; // 检查 MIE 位
}

static inline void set_PRIMASK(bool en) {
    if (en) {
        set_MSTATUS(get_MSTATUS() | MIE_BITMASK); // 设置 MIE 位
    } else {
        set_MSTATUS(get_MSTATUS() & ~(MIE_BITMASK)); // 清除 MIE 位
    }
}

[[nodiscard]] static inline uint32_t enter_critical() {
    uint32_t primask = get_PRIMASK();
    cpu_core::disable_irq();
    return primask;
}

static inline void exit_critical(uint32_t priority_mask) {
    cpu_core::set_PRIMASK(priority_mask);
}
}

#else
#warning "not supported currently"
#endif