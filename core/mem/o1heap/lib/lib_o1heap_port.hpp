#pragma once

#include "core/debug/debug.hpp"

// ---------------------------------------- BUILD CONFIGURATION OPTIONS ----------------------------------------

/// Define this macro to include build configuration header. This is an alternative to the -D compiler flag.
/// Usage example with CMake: "-DO1HEAP_CONFIG_HEADER=\"${CMAKE_CURRENT_SOURCE_DIR}/my_o1heap_config.h\""
#ifdef O1HEAP_CONFIG_HEADER
#    include O1HEAP_CONFIG_HEADER
#endif

/// The assertion macro defaults to the standard assert().
/// It can be overridden to manually suppress assertion checks or use a different error handling policy.


// template<typename ... Args>
#define O1HEAP_ASSERT(...) ymd::ASSERT(__VA_ARGS__)
#define O1HEAP_ASSUME_ALIGNED(ptr, n, ...) ({std::assume_aligned<n>(ptr);})
/// Allow usage of compiler intrinsics for branch annotation and CLZ.
#ifndef O1HEAP_USE_INTRINSICS
#    define O1HEAP_USE_INTRINSICS 1
#endif

/// Branch probability annotations are used to improve the worst case execution time (WCET). They are entirely optional.
#if O1HEAP_USE_INTRINSICS && !defined(O1HEAP_LIKELY)
#    if defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM)
// Intentional violation of MISRA: branch hinting macro cannot be replaced with a function definition.
#        define O1HEAP_LIKELY(x) __builtin_expect((x), 1)    // NOSONAR
#        define O1HEAP_UNLIKELY(x) __builtin_expect((x), 0)  // NOSONAR
#    endif
#endif
#ifndef O1HEAP_LIKELY
#    define O1HEAP_LIKELY(x) (x)
#endif
#ifndef O1HEAP_UNLIKELY
#    define O1HEAP_UNLIKELY(x) (x)
#endif

/// This option is used for testing only. Do not use in production.
#ifndef O1HEAP_PRIVATE
#    define O1HEAP_PRIVATE static inline
#endif

/// Count leading zeros (CLZ) is used for fast computation of binary logarithm (which needs to be done very often).
/// Most of the modern processors (including the embedded ones) implement dedicated hardware support for fast CLZ
/// computation, which is available via compiler intrinsics. The default implementation will automatically use
/// the intrinsics for some of the compilers; for others it will default to the slow software emulation,
/// which can be overridden by the user via O1HEAP_CONFIG_HEADER. The library guarantees that the argument is positive.
#if O1HEAP_USE_INTRINSICS && !defined(O1HEAP_CLZ)
#    if defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM)
#        define O1HEAP_CLZ __builtin_clzl
#    endif
#endif
#ifndef O1HEAP_CLZ
static constexpr uint_fast8_t O1HEAP_CLZ(const size_t x)
{
    O1HEAP_ASSERT(x > 0);
    size_t       t = ((size_t) 1U) << ((sizeof(size_t) * CHAR_BIT) - 1U);
    uint_fast8_t r = 0;
    while ((x & t) == 0)
    {
        t >>= 1U;
        r++;
    }
    return r;
}
#endif