#pragma once

#include "sys/core/sdk.h"


#ifdef ENABLE_IRQ_COUNTER
extern uint32_t irq_counters[];
#define COUNT_IRQ(irqn) (++irq_counters[irqn + 14])
#define GET_IRQ_COUNTER(irqn) irq_counters[irqn + 14]
#else
#define COUNT_IRQ(irqn) ((void)0)
#define GET_IRQ_COUNTER(irqn) 0
#endif

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

static inline bool cpu_enter_critical() {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void cpu_exit_critical(uint32_t priority_mask) {
    __set_PRIMASK(priority_mask);
}

// C++ only definitions

#ifdef __cplusplus

struct CriticalSectionContext {
    CriticalSectionContext(const CriticalSectionContext&) = delete;
    CriticalSectionContext(const CriticalSectionContext&&) = delete;
    void operator=(const CriticalSectionContext&) = delete;
    void operator=(const CriticalSectionContext&&) = delete;
    operator bool() { return true; };
    CriticalSectionContext() : mask_(cpu_enter_critical()) {}
    ~CriticalSectionContext() { cpu_exit_critical(mask_); }
    uint32_t mask_;
    bool exit_ = false;
};

#ifdef __clang__
#define CRITICAL_SECTION() for (CriticalSectionContext __critical_section_context; !__critical_section_context.exit_; __critical_section_context.exit_ = true)
#else
#define CRITICAL_SECTION() if (CriticalSectionContext __critical_section_context{})
#endif

#endif