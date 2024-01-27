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

#ifndef _FORCE_INLINE_
#define _FORCE_INLINE_ __fast_inline
#endif

#ifndef __interrupt_soft
#define __interrupt_soft extern "C" __attribute__((interrupt))
#endif

#ifndef __interrupt
#if defined(__riscv) && defined(WCH)
#define __interrupt extern "C" __attribute__((interrupt("WCH-Interrupt-fast")))
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

#ifndef _STR
#define _STR(m_x) #m_x
#define _MKSTR(m_x) _STR(m_x)
#endif

#define __STRX(m_index) #m_index
#define __STR(m_index) __STRX(m_index)

#endif