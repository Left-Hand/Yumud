#pragma once

#include "rts_support.hpp"
#include "_IQN.hpp"
#include "_IIQN.hpp"

// #define __IQMATH_USE_MY_IMPL

/* Common value defines. */

namespace __iqdetails{
static constexpr int16_t _q15_ln2          = 0x58b9;
static constexpr int16_t _q13_pi           = 0x6488;
static constexpr int16_t _q14_pi           = 0xc910;
static constexpr int16_t _q14_halfPi       = 0x6488;
static constexpr int16_t _q14_quarterPi    = 0x3244;
static constexpr int16_t _q15_halfPi       = 0xc910;
static constexpr int16_t _q15_quarterPi    = 0x6488;
static constexpr int16_t _q15_invRoot2     = 0x5a82;
static constexpr int16_t _q15_tanSmall     = 0x0021;
static constexpr int16_t _q15_pointOne     = 0x0ccd;
static constexpr int16_t _q15_oneTenth     = 0x0ccd;
static constexpr int32_t _iq28_twoPi       = 0x6487ed51;
static constexpr int32_t _iq29_pi          = 0x6487ed51;
static constexpr int32_t _iq29_halfPi      = 0x3243f6a8;
static constexpr int32_t _iq30_pi          = 0xc90fdaa2;
static constexpr int32_t _iq30_halfPi      = 0x6487ed51;
static constexpr int32_t _iq30_quarterPi   = 0x3243f6a8;
static constexpr int32_t _iq31_halfPi      = 0xc90fdaa2;
static constexpr int32_t _iq31_quarterPi   = 0x6487ed51;
static constexpr int32_t _iq31_invRoot2    = 0x5a82799a;
static constexpr int32_t _iq31_tanSmall    = 0x0020c49b;
static constexpr int32_t _iq31_ln2         = 0x58b90bfc;
static constexpr int32_t _iq31_twoThird    = 0x55555555;
static constexpr int32_t _iq31_pointOne    = 0x0ccccccd;
static constexpr int32_t _iq31_oneTenth    = 0x0ccccccd;
static constexpr int32_t _iq31_one         = 0x7fffffff;
}