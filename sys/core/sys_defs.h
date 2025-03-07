#pragma once

#ifndef __always_inline
#define __always_inline	__inline__ __attribute__((always_inline))
#endif

#ifndef __fast_access
#define __fast_access 
#endif

#ifndef __fast_inline
#define __fast_inline __always_inline __fast_access
#endif

#ifndef __no_inline
#define __no_inline __attribute__((__noinline__))
#endif


#ifndef __no_inline_constexpr
#ifdef __cplusplus
    #define __no_inline_constexpr __no_inline constexpr
#else
    #define __no_inline_constexpr __no_inline
#endif
#endif


#ifndef __fast_inline_consteval
#ifdef __cplusplus
    #define __fast_inline_consteval __fast_inline consteval
#else
    #define __fast_inline_consteval __fast_inline
#endif
#endif

#ifndef _FORCE_INLINE_
#define _FORCE_INLINE_ __fast_inline
#endif

#ifndef __interrupt_soft
#define __interrupt_soft __attribute__((interrupt))
#endif

#define EXECUTE(func, ...) if(likely(func)) func(__VA_ARGS__)

#ifndef __interrupt
#if defined(__riscv) && defined(WCH)
#define __interrupt __attribute__((interrupt("WCH-Interrupt-fast")))
#else
#define __interrupt __interrupt_soft
#endif
#endif

#ifndef __nop
#define __nop __asm volatile ("nop")
#endif

#ifndef CHECK_INIT
#define CHECK_INIT \
    { \
        static unsigned char inited = 0; \
        if (!inited) { \
            inited = 1; \
        }else{ \
            return; \
        }\
    } \

#endif

#define ARRSIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define ISSFR(ptr) (((uint32_t)ptr > 0x40000000))
#define ISRAM(ptr) ((!ISSFR(ptr)) && (((uint32_t)(ptr)) > 0x20000000))
#define ISROM(ptr) (((uint32_t)(ptr)) < 0x20000000)
#define ISALIGNED(ptr) ((((uint32_t)(ptr)) & 0x3) == 0)


#ifdef __cplusplus
#define scexpr static constexpr
#endif

#ifndef __nopn
#define __nopn(N) __asm__ volatile(".rept " #N "\n\t nop \n\t .endr \n\t":::)
#endif

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) x
#define unlikely(x) x
#endif

#ifdef __GNUC__
//#define FUNCTION_STR __PRETTY_FUNCTION__ - too annoying
#define FUNCTION_STR __FUNCTION__
#else
#define FUNCTION_STR __FUNCTION__
#endif

#define CONCAT(a, b) a ## b
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))

#define CHECK_TYPE(x, type) \
({ type __dummy; \
    typeof(x) __dummy2; \
    (void)(&__dummy == &__dummy2); \
    1; \
})

#define TYPE_CMP(a,b) __builtin_types_compatible_p(type_a, type_b);
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

#define BSWAP_8(x) ((decltype(x))((x) & 0xff))
#define BSWAP_16(x) ((decltype(x))((BSWAP_8(x) << 8) | BSWAP_8((x) >> 8)))
#define BSWAP_32(x) ((decltype(x))(((BSWAP_16(x) << 16) | BSWAP_16((x) >> 16))))
#define BSWAP_64(x) ((decltype(x))((BSWAP_32(x) << 32) | BSWAP_32((x) >> 32)))

#define VAR_AND_SIZE(x) x,sizeof(x)
#define PTR8_AND_SIZE(x) (const uint8_t *)&x, sizeof(x)
#define CHR8_AND_SIZE(x) (const char *)&x, sizeof(x)

#define BREAKPOINT __nopn(1);

#if defined(__riscv)
#define __HALT asm("csrrw zero, mstatus, zero");
#define DISABLE_INT   __asm volatile ("csrw 0x800, %0" : : "r" (0x6000) );
#define ENABLE_INT    __asm volatile ("csrw 0x800, %0" : : "r" (0x6088) );
#elif defined(__arm__)
    #if defined(__thumb__)
    #define __HALT asm("bkpt 0x00000000");
    #else
    #define HALT asm("swi 0x00000000");
    #endif

    #define DISABLE_INT
    #define ENABLE_INT
#else
#error "Not supported architecture"
#endif

// #define HALT __HALT; __builtin_unreachable();
#define HALT while(true);

#ifdef __cplusplus

#define DECLTYPE(...) decltype(__VA_ARGS__)

#define DELETE_COPY_AND_MOVE(type)\
type(const type & other) = delete;\
type(type && other) = delete;\


#endif