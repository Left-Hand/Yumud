#ifndef __SYS_DEFS_H__

#define __SYS_DEFS_H__

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

#ifndef _FORCE_INLINE_
#define _FORCE_INLINE_ __fast_inline
#endif

#ifndef __interrupt_soft
#define __interrupt_soft __attribute__((interrupt))
#endif

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

#define BSWAP_8(x) ((x) & 0xff)
#define BSWAP_16(x) ((BSWAP_8(x) << 8) | BSWAP_8((x) >> 8))
#define BSWAP_32(x) ((BSWAP_16(x) << 16) | BSWAP_16((x) >> 16))
#define BSWAP_64(x) ((BSWAP_32(x) << 32) | BSWAP_32((x) >> 32))

#endif