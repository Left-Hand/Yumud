#include "Critical.hpp"

#ifdef RISCV

bool __get_PRIMASK(void) {
    uint32_t mstatus = __get_MSTATUS();
    return (mstatus & (1 << 3)) ? 1 : 0; // 检查 MIE 位
}

void __set_PRIMASK(bool mask) {
    if (mask) {
        __set_MSTATUS(__get_MSTATUS() | (1 << 3)); // 设置 MIE 位
    } else {
        __set_MSTATUS(__get_MSTATUS() & ~(1 << 3)); // 清除 MIE 位
    }
}

#elif defined(ARM)

#endif


// #ifdef ENABLE_IRQ_COUNTER
// extern uint32_t irq_counters[];
// #define COUNT_IRQ(irqn) (++irq_counters[irqn + 14])
// #define GET_IRQ_COUNTER(irqn) irq_counters[irqn + 14]
// #else
// #define COUNT_IRQ(irqn) ((void)0)
// #define GET_IRQ_COUNTER(irqn) 0
// #endif
