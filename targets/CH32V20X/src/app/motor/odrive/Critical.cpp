#include "Critical.hpp"

#ifdef __riscv

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