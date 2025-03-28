#pragma once

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"
#include "core/math/realmath.hpp"

namespace ymd::foc{

scexpr uint chopper_freq = 50000;
scexpr uint foc_freq = chopper_freq / 2;

struct UvwValue{
    q20 u = {};
    q20 v = {};
    q20 w = {};

    q20 operator [](const size_t idx) const {
        return *(&u + idx);
    }

    q20 & operator [](const size_t idx){
        return *(&u + idx);
    }
};

struct UvwCurrent: public UvwValue{};
struct UvwVoltage: public UvwValue{};


struct DqValue{
    #pragma pack(push, 1)
    q20 d, q;
    #pragma pack(pop)

    q20 operator [](const size_t idx) const {
        return *(&d + idx);
    }

    q20 & operator [](const size_t idx){
        return *(&d + idx);
    }

    q20 length() const {
        return sqrt(d*d + q*q);
    }
};

struct DqCurrent: public DqValue{};
struct DqVoltage: public DqValue{};

struct AbValue{
    q20 a = {};
    q20 b = {};

    q20 operator [](const size_t idx) const {
        return *(&a + idx);
    }

    q20 & operator [](const size_t idx){
        return *(&a + idx);
    }

    q20 length() const {
        return sqrt(a*a + b*b);
    }
};

struct AbCurrent: public AbValue{};
struct AbVoltage: public AbValue{};


__fast_inline AbCurrent uvw_to_ab(const UvwCurrent & uvw){
    return {(uvw.u - ((uvw.v + uvw.w) >> 1)) * q16(2.0/3), (uvw.v - uvw.w) * q16(1.731 / 3)};
};

DqCurrent ab_to_dq(const AbCurrent & ab, const q16 rad);
DqVoltage ab_to_dq(const AbVoltage & ab, const q16 rad);

AbCurrent dq_to_ab(const DqCurrent & dq, const q16 rad);
AbVoltage dq_to_ab(const DqVoltage & dq, const q16 rad);

DqCurrent ab_to_dq(const AbCurrent & ab, const q16 s, const q16 c);
DqVoltage ab_to_dq(const AbVoltage & ab, const q16 s, const q16 c);

AbCurrent dq_to_ab(const DqCurrent & dq, const q16 s, const q16 c);
AbVoltage dq_to_ab(const DqVoltage & dq, const q16 s, const q16 c);

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
        return os << os.brackets<'('>() << 
            ab.a << os.splitter() << 
            ab.b << os.brackets<')'>();
    }
    
    inline OutputStream & operator << (OutputStream & os, const foc::DqValue & dq){
        return os << os.brackets<'('>() << 
            dq.d << os.splitter() << 
            dq.q << os.brackets<')'>();
    }

    inline OutputStream & operator << (OutputStream & os, const foc::UvwValue & uvw){
        return os << os.brackets<'('>() << 
            uvw.u << os.splitter() << 
            uvw.v <<  os.splitter() << 
            uvw.w << os.brackets<')'>();
    }
}

// struct TurnSolver{
//     uint16_t ta = 0;
//     uint16_t tb = 0;
//     real_t pa = 0;
//     real_t pb = 0;
//     real_t va = 0;
//     real_t vb = 0;
// };

// [[maybe_unused]] static real_t demo(uint milliseconds){
//     // using Vector2_t<real_t> = CubicInterpolation::Vector2_t<real_t>;
//     static TurnSolver turnSolver;
    
//     uint32_t turnCnt = milliseconds % 2667;
//     uint32_t turns = milliseconds / 2667;
    
//     scexpr real_t velPoints[7] = {
//         real_t(20)/360, real_t(20)/360, real_t(62.4)/360, real_t(62.4)/360, real_t(20.0)/360, real_t(20.0)/360, real_t(20.0)/360
//     };
    
//     scexpr real_t posPoints[7] = {
//         real_t(1.0f)/360,real_t(106.1f)/360,real_t(108.1f)/360, real_t(126.65f)/360, real_t(233.35f)/360,real_t(359.0f)/360,real_t(361.0f)/360
//     };

//     scexpr uint tickPoints[7] = {
//         0, 300, 400, 500, 2210, 2567, 2667 
//     };

//     int8_t i = 6;

//     while((turnCnt < tickPoints[i]) && (i > -1))
//         i--;
    
//     turnSolver.ta = tickPoints[i];
//     turnSolver.tb = tickPoints[i + 1];
//     auto dt = turnSolver.tb - turnSolver.ta;

//     turnSolver.va = velPoints[i];
//     turnSolver.vb = velPoints[i + 1];
    
//     turnSolver.pa = posPoints[i];
//     turnSolver.pb = posPoints[i + 1];
//     real_t dp = turnSolver.pb - turnSolver.pa;

//     real_t _t = ((real_t)(turnCnt  - turnSolver.ta) / dt);
//     real_t temp = (real_t)dt / 1000 / dp; 

//     real_t yt = 0;

//     if((i == 0) || (i == 2) || (i == 4))
//         yt = CubicInterpolation::forward(
//             Vector2_t<real_t>{real_t(0.4f), real_t(0.4f) * turnSolver.va * temp}, 
//             Vector2_t<real_t>(real_t(0.6f), real_t(1.0f) - real_t(0.4f)  * turnSolver.vb * temp), _t);
//     else
//         yt = _t;

//     real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;

//     return new_pos;
// }
