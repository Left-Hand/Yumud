#pragma once

#include <math.h>
#include "rts_support.h"

#if defined(__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__IQNexp)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#elif defined(__GNUC__) && !defined(__STATIC_INLINE)
#define __STATIC_INLINE inline __attribute__((always_inline))
#endif

/* Common value defines. */
#define _q15_ln2          0x58b9
#define _q13_pi           0x6488
#define _q14_pi           0xc910
#define _q14_halfPi       0x6488
#define _q14_quarterPi    0x3244
#define _q15_halfPi       0xc910
#define _q15_quarterPi    0x6488
#define _q15_invRoot2     0x5a82
#define _q15_tanSmall     0x0021
#define _q15_pointOne     0x0ccd
#define _q15_oneTenth     0x0ccd
#define _iq28_twoPi       0x6487ed51
#define _iq29_pi          0x6487ed51
#define _iq29_halfPi      0x3243f6a8
#define _iq30_pi          0xc90fdaa2
#define _iq30_halfPi      0x6487ed51
#define _iq30_quarterPi   0x3243f6a8
#define _iq31_halfPi      0xc90fdaa2
#define _iq31_quarterPi   0x6487ed51
#define _iq31_invRoot2    0x5a82799a
#define _iq31_tanSmall    0x0020c49b
#define _iq31_ln2         0x58b90bfc
#define _iq31_twoThird    0x55555555
#define _iq31_pointOne    0x0ccccccd
#define _iq31_oneTenth    0x0ccccccd
#define _iq31_one         0x7fffffff
