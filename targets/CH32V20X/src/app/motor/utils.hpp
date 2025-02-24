#pragma once

#include "sys/math/real.hpp"
#include "sys/stream/ostream.hpp"

namespace ymd::foc{

// scexpr uint chopper_freq = 32768;
scexpr uint chopper_freq = 50000;
scexpr uint foc_freq = 25000;

using Current = real_t;

struct UvwValue{
    #pragma pack(push, 1)
    real_t u, v, w;
    #pragma pack(pop)

    real_t operator [](const size_t idx) const {
        return *(&u + idx);
    }

    real_t & operator [](const size_t idx){
        return *(&u + idx);
    }
};

struct UvwCurrent: public UvwValue{};
struct UvwVoltage: public UvwValue{};


struct DqValue{
    #pragma pack(push, 1)
    real_t d, q;
    #pragma pack(pop)

    real_t operator [](const size_t idx) const {
        return *(&d + idx);
    }

    real_t & operator [](const size_t idx){
        return *(&d + idx);
    }

    real_t length() const {
        return sqrt(d*d + q*q);
    }
};

struct DqCurrent: public DqValue{};
struct DqVoltage: public DqValue{};

struct AbValue{
    #pragma pack(push, 1)
    real_t a, b;
    #pragma pack(pop)

    real_t operator [](const size_t idx) const {
        return *(&a + idx);
    }

    real_t & operator [](const size_t idx){
        return *(&a + idx);
    }

    real_t length() const {
        return sqrt(a*a + b*b);
    }
};

struct AbCurrent: public AbValue{};
struct AbVoltage: public AbValue{};


__inline AbCurrent uvw_to_ab(const UvwCurrent & uvw){
    return {(uvw.u - ((uvw.v + uvw.w) >> 1)) * real_t(2.0/3), (uvw.v - uvw.w) * real_t(1.731 / 3)};
};

DqCurrent ab_to_dq(const AbCurrent & ab, const real_t rad);
DqVoltage ab_to_dq(const AbVoltage & ab, const real_t rad);

AbCurrent dq_to_ab(const DqCurrent & dq, const real_t rad);
AbVoltage dq_to_ab(const DqVoltage & dq, const real_t rad);

void init_adc();
static __inline real_t sign_sqrt(const real_t x){
    return x < 0 ? -sqrt(-x) : sqrt(x);
};

static __inline real_t smooth(const real_t x){
    return x - sin(x);
}



}

namespace ymd{
    inline OutputStream & operator << (OutputStream & os, const foc::AbValue & ab){
        return os << '(' << ab.a << os.splitter() << ab.b << ')';
    }
    
    inline OutputStream & operator << (OutputStream & os, const foc::DqValue & dq){
        return os << '(' << dq.d << os.splitter() << dq.q << ')';
    }
    inline OutputStream & operator << (OutputStream & os, const foc::UvwValue & uvw){
        return os << '(' << uvw.u << os.splitter() << uvw.v <<  os.splitter() << uvw.w << ')';
    }
}