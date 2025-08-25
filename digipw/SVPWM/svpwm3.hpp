#pragma once

#include "core/math/realmath.hpp"

namespace ymd::digipw{

static constexpr std::array<q16, 3> SVM(
    const q16 alpha_dutycycle, 
    const q16 beta_dutycycle
){
    enum class Sector:uint8_t{
        _1 = 0b010,
        _2 = 0b000,
        _3 = 0b100,
        _4 = 0b101,
        _5 = 0b111,
        _6 = 0b011
    };

    constexpr q16 ONE_BY_SQRT3 = 1 / sqrt(3_r);
    constexpr q16 HALF_ONE = q16(0.5);

    const auto beta_by_sqrt3 = beta_dutycycle * ONE_BY_SQRT3;

    Sector sector {uint8_t(
        (  uint8_t(std::signbit(beta_by_sqrt3 + alpha_dutycycle)) << 2)
        | (uint8_t(std::signbit(beta_by_sqrt3 - alpha_dutycycle)) << 1)
        | (uint8_t(std::signbit(beta_dutycycle)))
    )};

    switch(sector){
        case Sector::_1:
        case Sector::_4:
        {

            const q16 a = (alpha_dutycycle - beta_by_sqrt3) >> 1;
            const q16 b = beta_by_sqrt3;

            const q16 u = (HALF_ONE + a + b);
            const q16 v = (HALF_ONE - a + b);
            const q16 w = (HALF_ONE - a - b);

            return {u, v, w};
        }

        case Sector::_2:
        case Sector::_5:
        {
            const q16 u = HALF_ONE + alpha_dutycycle;
            const q16 v = HALF_ONE + beta_by_sqrt3;
            const q16 w = HALF_ONE - beta_by_sqrt3;

            return {u, v, w};
        }

        case Sector::_3:
        case Sector::_6:
        {
            const q16 a = beta_by_sqrt3;
            const q16 b = (- alpha_dutycycle - beta_by_sqrt3) >> 1;

            const q16 u = (HALF_ONE - a - b);
            const q16 v = (HALF_ONE + a + b);
            const q16 w = (HALF_ONE - a + b);

            return {u, v, w};
        }

        default:
            __builtin_unreachable();
    }
}


struct SVPWM3{

    // const int bus_volt = 12;
    static constexpr q16 INV_SCALE = q16(1.0 / 12);

    // template<typename Inst>
    // void set_dq_volt(Inst & inst, const q16 dv, const q16 qv, const q16 rad){
    //     const auto c = cos(rad);
    //     const auto s = sin(rad);
    //     set_ab_volt(inst, dv * c - qv * s, dv * c + qv * s);
    // }

    template<typename Inst>
    static void set_ab_volt(Inst & inst, const q16 av, const q16 bv){
        set_ab_dutycycle(inst, av * INV_SCALE, bv * INV_SCALE);
    }

    template<typename Inst>
    static void set_ab_dutycycle(
        Inst & inst, 
        const q16 alpha_dutycycle, 
        const q16 beta_dutycycle
    ){
        const auto dutycycle = SVM(q16(alpha_dutycycle), q16(beta_dutycycle));
        inst.set_dutycycle(std::span(dutycycle));
    }
};


}