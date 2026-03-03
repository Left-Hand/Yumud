#pragma once

#ifdef __riscv
#include "riscv/ch32/exceptional_isr.hpp"
#elif defined(__arm__)
#error
#else
#error
#endif