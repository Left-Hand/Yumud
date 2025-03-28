#include "svpwm3.hpp"

namespace ymd::drivers{

std::tuple<real_t, real_t, real_t> SVM(const real_t alpha, const real_t beta){

    enum class Sector:uint8_t{
        _1 = 0b010,
        _2 = 0b000,
        _3 = 0b100,
        _4 = 0b101,
        _5 = 0b111,
        _6 = 0b011
    };

    static constexpr real_t one_by_sqrt3 = 1 / sqrt(3_r);
    static constexpr real_t half_one = real_t(0.5);

    const auto beta_by_sqrt3 = beta * one_by_sqrt3;

    Sector sector {uint8_t(
        (  uint8_t(std::signbit(beta_by_sqrt3 + alpha)) << 2)
        | (uint8_t(std::signbit(beta_by_sqrt3 - alpha)) << 1)
        | (uint8_t(std::signbit(beta)))
    )};

    switch(sector){
        case Sector::_1:
        case Sector::_4:
        {
            const real_t a = alpha - beta_by_sqrt3;
            const real_t b = beta_by_sqrt3 << 1;

            const real_t u = (1 + a + b) >> 1;
            const real_t v = (1 - a + b) >> 1;
            const real_t w = (1 - a - b) >> 1;

            return {u, v, w};
        }

        case Sector::_2:
        case Sector::_5:
        {
            const real_t u = half_one + alpha;
            const real_t v = half_one + beta_by_sqrt3;
            const real_t w = half_one - beta_by_sqrt3;

            return {u, v, w};
        }

        case Sector::_3:
        case Sector::_6:
        {
            const real_t a = beta_by_sqrt3 << 1;
            const real_t b = - alpha - beta_by_sqrt3;

            const real_t u = (1 - a - b) >> 1;
            const real_t v = (1 + a + b) >> 1;
            const real_t w = (1 - a + b) >> 1;

            return {u, v, w};
        }

        default:
            __builtin_unreachable();
    }
}

void SVPWM3::setAbDuty(const real_t alaph, const real_t beta){
    driver_ = SVM(real_t(alaph), real_t(beta));
}

}