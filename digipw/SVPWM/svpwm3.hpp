#pragma once

#include "core/math/realmath.hpp"
#include "digipw/prelude/abdq.hpp"

namespace ymd::digipw{
// 输入的幅值的临界模长不能超过sqrt(3)/2 此时恰好能使其中一路输入满占空比或零占空比
__attribute__((optimize("O3")))
static constexpr UvwCoord<iq16> SVM(
    const AlphaBetaCoord<iq16> alphabeta_dutycycle
){
    enum class Sector:uint8_t{
        _1 = 0b010,
        _2 = 0b000,
        _3 = 0b100,
        _4 = 0b101,
        _5 = 0b111,
        _6 = 0b011
    };

    constexpr iq16 ONE_BY_SQRT3 = math::inv_sqrt(3_iq16);
    constexpr iq16 HALF_ONE = iq16(0.5);

    const auto [alpha_dutycycle, beta_dutycycle] = alphabeta_dutycycle;
    const auto beta_dutycycle_by_sqrt3 = beta_dutycycle * ONE_BY_SQRT3;

    const auto sector = Sector{static_cast<uint8_t>(
        (  static_cast<uint8_t>(std::signbit(beta_dutycycle_by_sqrt3 + alpha_dutycycle)) << 2)
        | (static_cast<uint8_t>(std::signbit(beta_dutycycle_by_sqrt3 - alpha_dutycycle)) << 1)
        | (static_cast<uint8_t>(std::signbit(beta_dutycycle_by_sqrt3)))
    )};

    switch(sector){
        case Sector::_1:
        case Sector::_4:
        {

            const iq16 a = (alpha_dutycycle - beta_dutycycle_by_sqrt3) >> 1;
            const iq16 b = beta_dutycycle_by_sqrt3;

            const iq16 u = (HALF_ONE + a + b);
            const iq16 v = (HALF_ONE - a + b);
            const iq16 w = (HALF_ONE - a - b);

            return {u, v, w};
        }

        case Sector::_2:
        case Sector::_5:
        {
            const iq16 u = HALF_ONE + alpha_dutycycle;
            const iq16 v = HALF_ONE + beta_dutycycle_by_sqrt3;
            const iq16 w = HALF_ONE - beta_dutycycle_by_sqrt3;

            return {u, v, w};
        }

        case Sector::_3:
        case Sector::_6:
        {
            const iq16 a = beta_dutycycle_by_sqrt3;
            const iq16 b = (- alpha_dutycycle - beta_dutycycle_by_sqrt3) >> 1;

            const iq16 u = (HALF_ONE - a - b);
            const iq16 v = (HALF_ONE + a + b);
            const iq16 w = (HALF_ONE - a + b);

            return {u, v, w};
        }

        default:
            __builtin_unreachable();
    }

}


}