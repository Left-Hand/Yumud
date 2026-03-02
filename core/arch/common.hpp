#pragma once

#if defined(__riscv)
#include "riscv/riscv_critical.hpp"
#elif defined(__arm__)
#warning "unsupported currently"
#else
#warning "unsupported architecture"
#endif