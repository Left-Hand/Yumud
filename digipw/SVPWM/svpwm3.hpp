#pragma once

#include "core/math/realmath.hpp"
#include "digipw/prelude/abdq.hpp"

namespace ymd::digipw{


enum class SvmSector:uint8_t{
    _1 = 0b010,
    _2 = 0b000,
    _3 = 0b100,
    _4 = 0b101,
    _5 = 0b111,
    _6 = 0b011
};

struct alignas(16) [[nodiscard]] SvmIntermediate final{

    using Self = SvmIntermediate;
    using Sector = SvmSector;
    // static constexpr iq16 ONE_BY_SQRT3 = iq16::from_bits(0.57735026919 * (1ull << 16));
    static constexpr uq32 ONE_BY_SQRT3 = uq32::from_bits(0.57735026919 * (1ull << 32));
    static constexpr iq16 HALF_ONE = iq16(0.5);

    iq16 alpha_dutycycle;
    iq16 beta_dutycycle_by_sqrt3;
    Sector sector;

    __attribute__((optimize("O3"), hot, flatten))
    static constexpr Self from(const AlphaBetaCoord<iq16> alphabeta_dutycycle){
        const auto [alpha_dutycycle, beta_dutycycle] = alphabeta_dutycycle;
        const auto beta_dutycycle_by_sqrt3 = beta_dutycycle * ONE_BY_SQRT3;
    
        const auto sector = Sector{static_cast<uint8_t>(
            (  static_cast<uint8_t>(std::signbit(beta_dutycycle_by_sqrt3 + alpha_dutycycle)) << 2)
            | (static_cast<uint8_t>(std::signbit(beta_dutycycle_by_sqrt3 - alpha_dutycycle)) << 1)
            | (static_cast<uint8_t>(std::signbit(beta_dutycycle_by_sqrt3)))
        )};

        return Self{
            .alpha_dutycycle = alpha_dutycycle,
            .beta_dutycycle_by_sqrt3 = beta_dutycycle_by_sqrt3,
            .sector = sector
        };
    }

    __attribute__((optimize("O3"), hot, flatten))
    constexpr UvwCoord<iq16> to_uvw_dutycycle() const{
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
};


// 输入的幅值的临界模长不能超过sqrt(3)/2 此时恰好能使其中一路输入满占空比或零占空比
static constexpr UvwCoord<iq16> SVM(
    const AlphaBetaCoord<iq16> alphabeta_dutycycle
){
    return SvmIntermediate::from(alphabeta_dutycycle).to_uvw_dutycycle();
}

}