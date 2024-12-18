#pragma once

//#region arch
#ifdef __cplusplus 


#ifdef CH32V20X
#define ARCH_QKV4
#endif

#ifdef CH32V30X
#define ARCH_QKV4
#endif

#ifdef ARCH_QKV4
#include "arch/riscv/qkv4.hpp"
#endif
#endif
//#endregion


