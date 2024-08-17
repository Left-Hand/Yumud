/*******************************************************************
*
*    Author: Kareem Omar
*    kareem.h.omar@gmail.com
*    https://github.com/komrad36
*
*    Last updated Feb 15, 2021
*******************************************************************/

#pragma once

#include <cstdint>
#include "../sys/core/platform.h"

using int8_t  = int8_t;
using int16_t = int16_t;
using int32_t = int32_t;
using int64_t = int64_t;

using uint8_t  = uint8_t;
using uint16_t = uint16_t;
using uint32_t = uint32_t;
using uint64_t = uint64_t;

#define MAKE_BINARY_OP_HELPERS(op) \
friend auto operator op(const uint128_t& x, uint8_t   y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(const uint128_t& x, uint16_t  y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(const uint128_t& x, uint32_t  y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(const uint128_t& x, uint64_t  y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(const uint128_t& x, int8_t   y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(const uint128_t& x, int16_t  y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(const uint128_t& x, int32_t  y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(const uint128_t& x, int64_t  y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(const uint128_t& x, char y) { return operator op(x, (uint128_t)y); }  \
friend auto operator op(uint8_t   x, const uint128_t& y) { return operator op((uint128_t)x, y); }  \
friend auto operator op(uint16_t  x, const uint128_t& y) { return operator op((uint128_t)x, y); }  \
friend auto operator op(uint32_t  x, const uint128_t& y) { return operator op((uint128_t)x, y); }  \
friend auto operator op(uint64_t  x, const uint128_t& y) { return operator op((uint128_t)x, y); }  \
friend auto operator op(int8_t   x, const uint128_t& y) { return operator op((uint128_t)x, y); }  \
friend auto operator op(int16_t  x, const uint128_t& y) { return operator op((uint128_t)x, y); }  \
friend auto operator op(int32_t  x, const uint128_t& y) { return operator op((uint128_t)x, y); }  \
friend auto operator op(int64_t  x, const uint128_t& y) { return operator op((uint128_t)x, y); }  \
friend auto operator op(char x, const uint128_t& y) { return operator op((uint128_t)x, y); }

#define MAKE_BINARY_OP_HELPERS_FLOAT(op) \
friend auto operator op(const uint128_t& x, float  y) { return (float)x op y; }   \
friend auto operator op(const uint128_t& x, double y) { return (double)x op y; }  \
friend auto operator op(float  x, const uint128_t& y) { return x op (float)y; }    \
friend auto operator op(double x, const uint128_t& y) { return x op (double)y; }

#define MAKE_BINARY_OP_HELPERS_U64(op) \
friend uint128_t operator op(const uint128_t& x, uint8_t  n) { return operator op(x, (uint64_t)n); }    \
friend uint128_t operator op(const uint128_t& x, uint16_t n) { return operator op(x, (uint64_t)n); }    \
friend uint128_t operator op(const uint128_t& x, uint32_t n) { return operator op(x, (uint64_t)n); }    \
friend uint128_t operator op(const uint128_t& x, int8_t  n) { return operator op(x, (uint64_t)n); }    \
friend uint128_t operator op(const uint128_t& x, int16_t n) { return operator op(x, (uint64_t)n); }    \
friend uint128_t operator op(const uint128_t& x, int32_t n) { return operator op(x, (uint64_t)n); }    \
friend uint128_t operator op(const uint128_t& x, int64_t n) { return operator op(x, (uint64_t)n); }    \
friend uint128_t operator op(const uint128_t& x, const uint128_t& n) { return operator op(x, (uint64_t)n); }

class uint128_t
{
public:
    friend uint128_t DivMod(uint128_t n, uint128_t d, uint128_t& rem);

    uint128_t() = default;
    uint128_t(uint8_t    x) : m_lo(x), m_hi(0) {}
    uint128_t(uint16_t   x) : m_lo(x), m_hi(0) {}
    uint128_t(uint32_t   x) : m_lo(x), m_hi(0) {}
    uint128_t(uint64_t   x) : m_lo(x), m_hi(0) {}
    uint128_t(int8_t    x) : m_lo(int64_t(x)), m_hi(int64_t(x) >> 63) {}
    uint128_t(int16_t   x) : m_lo(int64_t(x)), m_hi(int64_t(x) >> 63) {}
    uint128_t(int32_t   x) : m_lo(int64_t(x)), m_hi(int64_t(x) >> 63) {}
    uint128_t(int64_t   x) : m_lo(int64_t(x)), m_hi(int64_t(x) >> 63) {}
    uint128_t(uint64_t hi, uint64_t lo) : m_lo(lo), m_hi(hi) {}

    // inexact values truncate, as per the Standard [conv.fpint]
    // passing values unrepresentable in the destination format is undefined behavior,
    // as per the Standard, but this implementation saturates
    uint128_t(float x);

    // inexact values truncate, as per the Standard [conv.fpint]
    // passing values unrepresentable in the destination format is undefined behavior,
    // as per the Standard, but this implementation saturates
    uint128_t(double x);

    uint128_t& operator+=(const uint128_t& x)
    {
        static_cast<void>(_addcarry_u64(_addcarry_u64(0, m_lo, x.m_lo, &m_lo), m_hi, x.m_hi, &m_hi));
        return *this;
    }

    friend uint128_t operator+(const uint128_t& x, const uint128_t& y)
    {
        uint128_t ret;
        static_cast<void>(_addcarry_u64(_addcarry_u64(0, x.m_lo, y.m_lo, &ret.m_lo), x.m_hi, y.m_hi, &ret.m_hi));
        return ret;
    }

    MAKE_BINARY_OP_HELPERS(+);
    MAKE_BINARY_OP_HELPERS_FLOAT(+);

    uint128_t& operator-=(const uint128_t& x)
    {
        static_cast<void>(_subborrow_u64(_subborrow_u64(0, m_lo, x.m_lo, &m_lo), m_hi, x.m_hi, &m_hi));
        return *this;
    }

    friend uint128_t operator-(const uint128_t& x, const uint128_t& y)
    {
        uint128_t ret;
        static_cast<void>(_subborrow_u64(_subborrow_u64(0, x.m_lo, y.m_lo, &ret.m_lo), x.m_hi, y.m_hi, &ret.m_hi));
        return ret;
    }

    MAKE_BINARY_OP_HELPERS(-);
    MAKE_BINARY_OP_HELPERS_FLOAT(-);

    uint128_t& operator*=(const uint128_t& x)
    {
        // ab * cd
        // ==
        // (2^64*a + b) * (2^64*c + d)
        // if a*c == e, a*d == f, b*c == g, b*d == h
        // |ee|ee|  |  |
        // |  |fg|fg|  |
        // |  |  |hh|hh|

        uint64_t hHi;
        const uint64_t hLo = _umul128(m_lo, x.m_lo, &hHi);
        m_hi = hHi + m_hi * x.m_lo + m_lo * x.m_hi;
        m_lo = hLo;
        return *this;
    }

    friend uint128_t operator*(const uint128_t& x, const uint128_t& y)
    {
        uint128_t ret;
        uint64_t hHi;
        ret.m_lo = _umul128(x.m_lo, y.m_lo, &hHi);
        ret.m_hi = hHi + y.m_hi * x.m_lo + y.m_lo * x.m_hi;
        return ret;
    }

    MAKE_BINARY_OP_HELPERS(*);
    MAKE_BINARY_OP_HELPERS_FLOAT(*);

    uint128_t& operator/=(const uint128_t& x)
    {
        uint128_t rem;
        *this = DivMod(*this, x, rem);
        return *this;
    }

    friend uint128_t operator/(const uint128_t& x, const uint128_t& y)
    {
        uint128_t rem;
        return DivMod(x, y, rem);
    }

    MAKE_BINARY_OP_HELPERS(/);
    MAKE_BINARY_OP_HELPERS_FLOAT(/);

    uint128_t& operator%=(const uint128_t& x)
    {
        static_cast<void>(DivMod(*this, x, *this));
        return *this;
    }

    friend uint128_t operator%(const uint128_t& x, const uint128_t& y)
    {
        uint128_t ret;
        static_cast<void>(DivMod(x, y, ret));
        return ret;
    }

    MAKE_BINARY_OP_HELPERS(%);

    uint128_t& operator&=(const uint128_t& x)
    {
        m_hi &= x.m_hi;
        m_lo &= x.m_lo;
        return *this;
    }

    friend uint128_t operator&(const uint128_t& x, const uint128_t& y)
    {
        return uint128_t(x.m_hi & y.m_hi, x.m_lo & y.m_lo);
    }

    MAKE_BINARY_OP_HELPERS(&);

    uint128_t& operator|=(const uint128_t& x)
    {
        m_hi |= x.m_hi;
        m_lo |= x.m_lo;
        return *this;
    }

    friend uint128_t operator|(const uint128_t& x, const uint128_t& y)
    {
        return uint128_t(x.m_hi | y.m_hi, x.m_lo | y.m_lo);
    }

    MAKE_BINARY_OP_HELPERS(|);

    uint128_t& operator^=(const uint128_t& x)
    {
        m_hi ^= x.m_hi;
        m_lo ^= x.m_lo;
        return *this;
    }

    friend uint128_t operator^(const uint128_t& x, const uint128_t& y)
    {
        return uint128_t(x.m_hi ^ y.m_hi, x.m_lo ^ y.m_lo);
    }

    MAKE_BINARY_OP_HELPERS(^);

    uint128_t& operator>>=(uint64_t n)
    {
        const uint64_t lo = __shiftright128(m_lo, m_hi, (uint8_t)n);
        const uint64_t hi = m_hi >> (n & 63ULL);

        m_lo = n & 64 ? hi : lo;
        m_hi = n & 64 ? 0  : hi;

        return *this;
    }

    friend uint128_t operator>>(const uint128_t& x, uint64_t n)
    {
        uint128_t ret;

        const uint64_t lo = __shiftright128(x.m_lo, x.m_hi, (uint8_t)n);
        const uint64_t hi = x.m_hi >> (n & 63ULL);

        ret.m_lo = n & 64 ? hi : lo;
        ret.m_hi = n & 64 ? 0  : hi;

        return ret;
    }

    MAKE_BINARY_OP_HELPERS_U64(>>);

    uint128_t& operator<<=(uint64_t n)
    {
        const uint64_t hi = __shiftleft128(m_lo, m_hi, (uint8_t)n);
        const uint64_t lo = m_lo << (n & 63ULL);

        m_hi = n & 64 ? lo : hi;
        m_lo = n & 64 ? 0 : lo;

        return *this;
    }

    friend uint128_t operator<<(const uint128_t& x, uint64_t n)
    {
        uint128_t ret;

        const uint64_t hi = __shiftleft128(x.m_lo, x.m_hi, (uint8_t)n);
        const uint64_t lo = x.m_lo << (n & 63ULL);

        ret.m_hi = n & 64 ? lo : hi;
        ret.m_lo = n & 64 ? 0 : lo;

        return ret;
    }

    MAKE_BINARY_OP_HELPERS_U64(<<);

    friend uint128_t operator~(const uint128_t& x)
    {
        return uint128_t(~x.m_hi, ~x.m_lo);
    }

    friend uint128_t operator+(const uint128_t& x)
    {
        return x;
    }

    friend uint128_t operator-(const uint128_t& x)
    {
        uint128_t ret;
        static_cast<void>(_subborrow_u64(_subborrow_u64(0, 0, x.m_lo, &ret.m_lo), 0, x.m_hi, &ret.m_hi));
        return ret;
    }

    uint128_t& operator++()
    {
        operator+=(1);
        return *this;
    }

    uint128_t operator++(int)
    {
        const uint128_t x = *this;
        operator++();
        return x;
    }

    uint128_t& operator--()
    {
        operator-=(1);
        return *this;
    }

    uint128_t operator--(int)
    {
        const uint128_t x = *this;
        operator--();
        return x;
    }

    friend bool operator<(const uint128_t& x, const uint128_t& y)
    {
        uint64_t unusedLo, unusedHi;
        return _subborrow_u64(_subborrow_u64(0, x.m_lo, y.m_lo, &unusedLo), x.m_hi, y.m_hi, &unusedHi);
    }
    MAKE_BINARY_OP_HELPERS(<);
    MAKE_BINARY_OP_HELPERS_FLOAT(<);

    friend bool operator>(const uint128_t& x, const uint128_t& y) { return y < x; }
    MAKE_BINARY_OP_HELPERS(>);
    MAKE_BINARY_OP_HELPERS_FLOAT(>);

    friend bool operator<=(const uint128_t& x, const uint128_t& y) { return !(x > y); }
    MAKE_BINARY_OP_HELPERS(<=);
    MAKE_BINARY_OP_HELPERS_FLOAT(<=);

    friend bool operator>=(const uint128_t& x, const uint128_t& y) { return !(x < y); }
    MAKE_BINARY_OP_HELPERS(>=);
    MAKE_BINARY_OP_HELPERS_FLOAT(>=);

    friend bool operator==(const uint128_t& x, const uint128_t& y)
    {
        return !((x.m_hi ^ y.m_hi) | (x.m_lo ^ y.m_lo));
    }
    MAKE_BINARY_OP_HELPERS(==);
    MAKE_BINARY_OP_HELPERS_FLOAT(==);

    friend bool operator!=(const uint128_t& x, const uint128_t& y) { return !(x == y); }
    MAKE_BINARY_OP_HELPERS(!=);
    MAKE_BINARY_OP_HELPERS_FLOAT(!=);

    explicit operator bool() const { return m_hi | m_lo; }

    operator uint8_t () const { return (uint8_t) m_lo; }
    operator uint16_t() const { return (uint16_t)m_lo; }
    operator uint32_t() const { return (uint32_t)m_lo; }
    operator uint64_t() const { return (uint64_t)m_lo; }

    operator int8_t () const { return (int8_t) m_lo; }
    operator int16_t() const { return (int16_t)m_lo; }
    operator int32_t() const { return (int32_t)m_lo; }
    operator int64_t() const { return (int64_t)m_lo; }

    operator char() const { return (char)m_lo; }

    // rounding method is implementation-defined as per the Standard [conv.fpint]
    // this implementation performs IEEE 754-compliant "round half to even" rounding to nearest,
    // regardless of the current FPU rounding mode, which matches the behavior of clang and GCC
    operator float() const;

    // rounding method is implementation-defined as per the Standard [conv.fpint]
    // this implementation performs IEEE 754-compliant "round half to even" rounding to nearest,
    // regardless of the current FPU rounding mode, which matches the behavior of clang and GCC
    operator double() const;

    // caller is responsible for ensuring that buf has space for the uint128_t AND the null terminator
    // that follows, in the given output base.
    // Common bases and worst-case size requirements:
    // Base  2: 129 bytes (128 + null terminator)
    // Base  8:  44 bytes ( 43 + null terminator)
    // Base 10:  40 bytes ( 39 + null terminator)
    // Base 16:  33 bytes ( 32 + null terminator)
    void ToString(char* buf, uint64_t base = 10) const;

private:
    uint64_t m_lo;
    uint64_t m_hi;
};

#undef MAKE_BINARY_OP_HELPERS
#undef MAKE_BINARY_OP_HELPERS_FLOAT
#undef MAKE_BINARY_OP_HELPERS_U64

std::ostream& operator<<(std::ostream& os, const uint128_t& x);