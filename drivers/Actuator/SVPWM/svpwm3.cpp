#include "svpwm3.hpp"

namespace ymd::drivers{

std::tuple<real_t, real_t, real_t> SVM(const real_t alpha, const real_t beta){

    enum Sector:uint8_t{
        _1 = 0b010,
        _2 = 0b000,
        _3 = 0b100,
        _4 = 0b101,
        _5 = 0b111,
        _6 = 0b011
    };

    scexpr real_t one_by_sqrt3 = real_t(1.0 / 1.7320508075688772);
    scexpr real_t half = real_t(0.5);

    const auto beta_by_sqrt3 = beta * one_by_sqrt3;

    const auto p1 = std::signbit(beta);
    const auto p2 = std::signbit(beta_by_sqrt3 - alpha);
    const auto p3 = std::signbit(beta_by_sqrt3 + alpha);

    // const auto a = beta_by_sqrt3 + alpha;
    Sector sector = Sector(
        p1 | (p2 << 1) | (p3 << 2)
    );

    switch(sector){
        default:
            //will not reach here
            // return {0,0,0};
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
            const real_t u = half + alpha;
            const real_t v = half + beta_by_sqrt3;
            const real_t w = half - beta_by_sqrt3;

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
    }
}

// void SVPWM3::setDuty(const real_t duty, const real_t rad) {
//     driver_ = SVM(real_t(duty * cos(rad)), real_t(duty * sin(rad)));
// }


void SVPWM3::setAbDuty(const real_t alaph, const real_t beta){
    driver_ = SVM(real_t(alaph), real_t(beta));
}

}