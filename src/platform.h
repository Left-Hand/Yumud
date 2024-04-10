#ifndef __PLATFORM_H__

#define __PLATFORM_H__

#include "defines/defs.h"

#include <stdint.h>
#include <stdbool.h>

#include <ch32v20x.h>

/**
 * User-defined Literals
 *  usage:
 *
 *   uint32_t = test = 10_MHz; // --> 10000000
 */



// __fast_inline uint64_t operator"" _MHz(const uint64_t & x)
// {
//     return x * 1000 * 1000;
// }

// __fast_inline uint64_t operator"" _GHz(const uint64_t & x)
// {
//     return x * 1000 * 1000 * 1000;
// }

// __fast_inline uint64_t operator"" _kBit(const uint64_t & x)
// {
//     return x * 1024;
// }

// __fast_inline uint64_t operator"" _MBit(const uint64_t & x)
// {
//     return x * 1024 * 1024;
// }

// __fast_inline uint64_t operator"" _GBit(const uint64_t & x)
// {
//     return x * 1024 * 1024 * 1024;
// }

// __fast_inline uint64_t operator"" _kB(const uint64_t & x)
// {
//     return x * 1024;
// }

// __fast_inline uint64_t operator"" _MB(const uint64_t & x)
// {
//     return x * 1024 * 1024;
// }

// __fast_inline uint64_t operator"" _GB(const uint64_t & x)
// {
//     return x * 1024 * 1024 * 1024;
// }



#endif