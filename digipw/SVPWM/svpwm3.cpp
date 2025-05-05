#include "svpwm3.hpp"

namespace ymd::digipw{

std::tuple<real_t, real_t, real_t> SVM(const real_t alpha, const real_t beta){
    enum class Sector:uint8_t{
        _1 = 0b010,
        _2 = 0b000,
        _3 = 0b100,
        _4 = 0b101,
        _5 = 0b111,
        _6 = 0b011
    };

    static constexpr q16 ONE_BY_SQRT3 = 1 / sqrt(3_r);
    static constexpr q16 HALF_ONE = q16(0.5);

    const auto beta_by_sqrt3 = beta * ONE_BY_SQRT3;

    Sector sector {uint8_t(
        (  uint8_t(std::signbit(beta_by_sqrt3 + alpha)) << 2)
        | (uint8_t(std::signbit(beta_by_sqrt3 - alpha)) << 1)
        | (uint8_t(std::signbit(beta)))
    )};

    switch(sector){
        case Sector::_1:
        case Sector::_4:
        {
            // const q16 a = alpha - beta_by_sqrt3;
            // const q16 b = beta_by_sqrt3 << 1;

            // const q16 u = (1 + a + b) >> 1;
            // const q16 v = (1 - a + b) >> 1;
            // const q16 w = (1 - a - b) >> 1;

            const q16 a = (alpha - beta_by_sqrt3) >> 1;
            const q16 b = beta_by_sqrt3;

            const q16 u = (HALF_ONE + a + b);
            const q16 v = (HALF_ONE - a + b);
            const q16 w = (HALF_ONE - a - b);

            return {u, v, w};
        }

        case Sector::_2:
        case Sector::_5:
        {
            const q16 u = HALF_ONE + alpha;
            const q16 v = HALF_ONE + beta_by_sqrt3;
            const q16 w = HALF_ONE - beta_by_sqrt3;

            return {u, v, w};
        }

        case Sector::_3:
        case Sector::_6:
        {
            const q16 a = beta_by_sqrt3;
            const q16 b = (- alpha - beta_by_sqrt3) >> 1;

            const q16 u = (HALF_ONE - a - b);
            const q16 v = (HALF_ONE + a + b);
            const q16 w = (HALF_ONE - a + b);

            return {u, v, w};
        }

        default:
            __builtin_unreachable();
    }
}

void SVPWM3::set_ab_duty(const real_t alaph, const real_t beta){
    driver_ = SVM(real_t(alaph), real_t(beta));
}

}