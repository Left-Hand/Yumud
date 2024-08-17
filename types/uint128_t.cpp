/*******************************************************************
*
*    Author: Kareem Omar
*    kareem.h.omar@gmail.com
*    https://github.com/komrad36
*
*    Last updated Feb 15, 2021
*******************************************************************/

#include <cstdint>
#include <intrin.h>
#include <iosfwd>
#include "uint128_t.h"

using I8 = int8_t;
using I16 = int16_t;
using I32 = int32_t;
using I64 = int64_t;

using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;

static inline bool FitsHardwareDivL(U64 nHi, U64 nLo, U64 d)
{
    return !(nHi | (d >> 32)) && nLo < (d << 32);
}

static inline U64 HardwareDivL(U64 n, U64 d, U64& rem)
{
    U32 rLo;
    const U32 qLo = _udiv64(n, U32(d), &rLo);
    rem = rLo;
    return qLo;
}

static inline U64 HardwareDivQ(U64 nHi, U64 nLo, U64 d, U64& rem)
{
    nLo = _udiv128(nHi, nLo, d, &nHi);
    rem = nHi;
    return nLo;
}

static inline bool IsPow2(U64 hi, U64 lo)
{
    const U64 T = hi | lo;
    return !((hi & lo) | (T & (T - 1)));
}

static inline U64 CountTrailingZeros(U64 hi, U64 lo)
{
    const U64 nLo = _tzcnt_u64(lo);
    const U64 nHi = 64ULL + _tzcnt_u64(hi);
    return lo ? nLo : nHi;
}

static inline U64 CountLeadingZeros(U64 hi, U64 lo)
{
    const U64 nLo = 64ULL + _lzcnt_u64(lo);
    const U64 nHi = _lzcnt_u64(hi);
    return hi ? nHi : nLo;
}

static inline uint128_t MaskBitsBelow(U64 hi, U64 lo, U64 n)
{
    return uint128_t(_bzhi_u64(hi, U32(n < 64 ? 0 : n - 64)), _bzhi_u64(lo, U32(n)));
}

uint128_t DivMod(uint128_t N, uint128_t D, uint128_t& rem)
{
    if (D > N)
    {
        rem = N;
        return 0;
    }

    U64 nHi = N.m_hi;
    U64 nLo = N.m_lo;
    U64 dHi = D.m_hi;
    U64 dLo = D.m_lo;

    if (IsPow2(dHi, dLo))
    {
        const U64 n = CountTrailingZeros(dHi, dLo);
        rem = MaskBitsBelow(nHi, nLo, n);
        return N >> n;
    }

    if (!dHi)
    {
        if (nHi < dLo)
        {
            U64 remLo;
            U64 Q;
            if (FitsHardwareDivL(nHi, nLo, dLo))
                Q = HardwareDivL(nLo, dLo, remLo);
            else
                Q = HardwareDivQ(nHi, nLo, dLo, remLo);
            rem = remLo;
            return Q;
        }

        U64 remLo;
        const U64 qHi = HardwareDivQ(0, nHi, dLo, remLo);
        const U64 qLo = HardwareDivQ(remLo, nLo, dLo, remLo);
        rem = remLo;
        return uint128_t(qHi, qLo);
    }

    U64 n = _lzcnt_u64(dHi) - _lzcnt_u64(nHi);

    dHi = __shiftleft128(dLo, dHi, U8(n));
    dLo <<= n;

    U64 Q = 0;
    ++n;

    do
    {
        U64 tLo, tHi;
        unsigned char carry = _subborrow_u64(_subborrow_u64(0, nLo, dLo, &tLo), nHi, dHi, &tHi);
        nLo = !carry ? tLo : nLo;
        nHi = !carry ? tHi : nHi;
        Q = (Q << 1) + !carry;
        dLo = __shiftright128(dLo, dHi, 1);
        dHi >>= 1;
    } while (--n);

    rem = uint128_t(nHi, nLo);
    return Q;
}

uint128_t::uint128_t(float x)
{
    const U32 bits = U32(_mm_cvtsi128_si32(_mm_castps_si128(_mm_set_ss(x))));
    const U32 s = bits >> 31;

    // technically UB but let's be nice
    if (s)
    {
        m_hi = m_lo = 0ULL;
        return;
    }

    const U32 e = (bits >> 23) - 127;
    const U32 m = (bits & ((1U << 23) - 1U)) | (1U << 23);

    // again, technically UB but let's be nice
    if (e >= 128)
    {
        m_hi = m_lo = ~0ULL;
        return;
    }

    if (e >= 23)
        *this = uint128_t(m) << (e - 23);
    else
        *this = m >> (23 - e);
}

uint128_t::uint128_t(double x)
{
    const U64 bits = U64(_mm_cvtsi128_si64(_mm_castpd_si128(_mm_set_sd(x))));
    const U64 s = bits >> 63;

    // technically UB but let's be nice
    if (s)
    {
        m_hi = m_lo = 0ULL;
        return;
    }

    const U64 e = (bits >> 52) - 1023;
    const U64 m = (bits & ((1ULL << 52) - 1ULL)) | (1ULL << 52);

    // again, technically UB but let's be nice
    if (e >= 128)
    {
        m_hi = m_lo = ~0ULL;
        return;
    }

    if (e >= 52)
        *this = uint128_t(m) << (e - 52);
    else
        *this = m >> (52 - e);
}

uint128_t::operator float() const
{
    if (!*this)
        return 0.0f;

    const U32 numBits = 128U - U32(CountLeadingZeros(m_hi, m_lo));

    U32 bits;

    if (numBits <= 24)
    {
        const U32 m = (U32(m_lo) << (24 - numBits)) & ~(1U << 23);
        const U32 e = numBits + 126;
        bits = (e << 23) | m;
    }
    else
    {
        const U32 s = numBits - 24;
        const U32 m = U32(*this >> s) & ~(1U << 23);
        const U32 G = U32(*this >> (s - 1));
        const U32 R = U32(bool(MaskBitsBelow(m_hi, m_lo, s < 2 ? 0 : s - 2)));
        const U32 e = numBits + 126;
        bits = ((e << 23) | m) + (G & (R | m) & 1U);
    }

    return _mm_cvtss_f32(_mm_castsi128_ps(_mm_cvtsi32_si128((I32)bits)));
}

uint128_t::operator double() const
{
    if (!*this)
        return 0.0;

    const U64 numBits = 128ULL - CountLeadingZeros(m_hi, m_lo);

    U64 bits;

    if (numBits <= 53)
    {
        const U64 m = (m_lo << (53 - numBits)) & ~(1ULL << 52);
        const U64 e = numBits + 1022;
        bits = (e << 52) | m;
    }
    else
    {
        const U64 s = numBits - 53;
        const U64 m = U64(*this >> s) & ~(1ULL << 52);
        const U64 G = U64(*this >> (s - 1));
        const U64 R = U64(bool(MaskBitsBelow(m_hi, m_lo, s < 2 ? 0 : s - 2)));
        const U64 e = numBits + 1022;
        bits = ((e << 52) | m) + (G & (R | m) & 1ULL);
    }

    return _mm_cvtsd_f64(_mm_castsi128_pd(_mm_cvtsi64_si128((I64)bits)));
}

void uint128_t::ToString(char* buf, U64 base/* = 10*/) const
{
    U64 i = 0;
    if (base >= 2 && base <= 36)
    {
        uint128_t n = *this;
        uint128_t r, b = base;
        do
        {
            n = DivMod(n, b, r);
            const char c(r);
            buf[i++] = c + (c >= 10 ? '7' : '0');
        } while (n);

        for (U64 j = 0; j < (i >> 1); ++j)
        {
            const char t = buf[j];
            buf[j] = buf[i - j - 1];
            buf[i - j - 1] = t;
        }
    }
    buf[i] = '\0';
}

std::ostream& operator<<(std::ostream& os, const uint128_t& x)
{
    char buf[40];
    x.ToString(buf);
    os << buf;
    return os;
}

const char* NatVisStr_DebugOnly(const uint128_t& x)
{
    static char buf[40];
    x.ToString(buf);
    return buf;
}