#pragma once

#include <cstdint>
#include <cstddef>

namespace nvcv2::backend{

void bitwise_not_inplace_softw(uint8_t * dst, const size_t len);

#if 0
#ifdef __riscv
void bitwise_not_inplace_rvvext(uint8_t * dst, const size_t len);
#endif
#endif
}