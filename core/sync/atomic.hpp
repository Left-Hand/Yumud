#pragma once

#if defined(__riscv) && defined(__riscv_atomic) 
#include "core/arch/riscv/atomic.hpp"

// namespace ymd::sync{
// // using Atomic = std::atomic
// }
#else
#error "This header is only supported on RISC-V platforms with atomic support"
#endif