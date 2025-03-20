#pragma once

#include "core/sdk.hpp"




bool __get_PRIMASK(void);
void __set_PRIMASK(bool mask);

static inline bool cpu_enter_critical() {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void cpu_exit_critical(uint32_t priority_mask) {
    __set_PRIMASK(priority_mask);
}


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


#define CRITICAL_SECTION() if (CriticalSectionContext __critical_section_context{})