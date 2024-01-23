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


#endif