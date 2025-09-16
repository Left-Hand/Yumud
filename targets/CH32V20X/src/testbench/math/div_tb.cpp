
//bionukg @ https://zhuanlan.zhihu.com/p/705601523


#include "../tb.h"

#include "core/math/int/int_t.hpp"
#include "core/debug/debug.hpp"


#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

static inline constexpr auto get_m_a_s_udiv32(uint32_t Divisor){
    const uint32_t n = std::countr_zero(Divisor);
    const uint32_t _t = Divisor >> n;
    struct
    {
        uint32_t a;
        uint32_t m;
        uint32_t s;
    } ret{};
    if (Divisor >= uint(1 << 31))
    {
        ret.m = 1;
        ret.s = 32;
        ret.a = uint32_t() - Divisor;
    }
    else if (_t == 1)
    {
        ret.m = 1;
        ret.s = n;
        ret.a = 0;
    }
    else
    {
        ///* Generate m, s for algorithm 0. Based on: Granlund, T.; Montgomery,
        // P.L.: "Division by Invariant Integers using Multiplication."
        // SIGPLAN Notices, Vol. 29, June 1994, page 61.
        //*/
        // l = log2(_t) + 1;
        // j = (((U64)(0xffffffff)) % ((U64)(_t)));
        // k = (((U64)(1)) << (32 + l)) / ((U64)(0xffffffff - j));
        // m_low = (((U64)(1)) << (32 + l)) / _t;
        // m_high = ((((U64)(1)) << (32 + l)) + k) / _t;

        constexpr uint64_t u32_max = uint32_t(~uint32_t(0));
        constexpr uint64_t one = 1;
        int l = 32 - std::countl_zero(_t); // log2(_t) + 1;
        uint64_t j = u32_max % uint64_t(_t);
        uint64_t k = (one << (32 + l)) / (u32_max - j);
        uint64_t m_lo = (one << (32 + l)) / _t;
        uint64_t m_hi = ((one << (32 + l)) + k) / _t;

        // while (((m_low >> 1) < (m_high >> 1)) && (l > 0)) {
        //     m_low = m_low >> 1;
        //     m_high = m_high >> 1;
        //     l = l - 1;
        // }
        while (((m_lo >> 1) < (m_hi >> 1)) && (l > 0))
        {
            m_lo >>= 1;
            m_hi >>= 1;
            l -= 1;
        }
        // if ((m_high >> 32) == 0) {
        //     m = ((U32)(m_high));
        //     s = l;
        //     a = 0;
        // }
        if ((m_hi >> 32) == 0)
        {
            ret.m = uint32_t(m_hi);
            ret.s = l;
            ret.a = 0;
        }
        ///* Generate m and s for algorithm 1. Based on: Magenheimer, D.J.; et al:
        //"Integer Multiplication and Division on the HP Precision Architecture."
        // IEEE Transactions on Computers, Vol. 37, No. 8, August 1988, page 980.*/
        // else {
        //    s = log2(_t);
        //    m_low = (((U64)(1)) << (32 + s)) / ((U64)(_t));
        //    r = ((U32)((((U64)(1)) << (32 + s)) % ((U64)(_t))));
        //    m = (r < ((t >> 1) + 1)) ? ((U32)(m_low)) : ((U32)(m_low)) + 1;
        //    a = 1;
        //}
        else
        {
            ret.s = 31 - std::countl_zero(_t);
            m_lo = (one << (32 + ret.s)) / uint64_t(_t);
            uint32_t r = (uint32_t(one << (32 + ret.s))) % uint64_t(_t);
            ret.m = (r < ((_t >> 1) + 1)) ? (uint32_t(m_lo)) : (uint32_t(m_lo)) + 1;
            ret.a = 1;
        }
        ///* Reduce multiplier for either algorithm to smallest possible.*/
        // while (!(m & 1)) {
        //     m = m >> 1;
        //     s--;
        // }
        {
            const int cntrz_m = std::countr_zero(ret.m);
            ret.s -= cntrz_m;
            ret.m >>= cntrz_m;
        }
        ///* Adjust multiplier for reduction of even divisors. */
        // s += n;
        ret.s += n + 32;
    }
    return ret;
};

static inline constexpr auto udivc(uint32_t Divisor){
     return [m_a_s = get_m_a_s_udiv32(Divisor)](uint32_t dividend) -> uint32_t
     {
       return ((uint64_t(dividend) + m_a_s.a) * m_a_s.m) >> (m_a_s.s);
     };
}

void div_tb() {
    DEBUGGER_INST.init({576000});

    // i8 a = {0};

    // DEBUG_PRINTLN(a);    
}
