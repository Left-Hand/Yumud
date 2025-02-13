#include "svpwm3.hpp"

namespace ymd::drivers{

std::tuple<iq_t, iq_t, iq_t> SVM(const iq_t alpha, const iq_t beta){

    enum Sector:uint8_t{
        _1 = 0b010,
        _2 = 0b000,
        _3 = 0b100,
        _4 = 0b101,
        _5 = 0b111,
        _6 = 0b011
    };

    scexpr iq_t one_by_sqrt3 = iq_t(1.0 / 1.7320508075688772);
    scexpr iq_t half = iq_t(0.5);

    const auto beta_by_sqrt3 = beta * one_by_sqrt3;

    const auto p1 = std::signbit(beta);
    const auto p2 = std::signbit(beta_by_sqrt3 - alpha);
    const auto p3 = std::signbit(beta_by_sqrt3 + alpha);

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
            const iq_t a = alpha - beta_by_sqrt3;
            const iq_t b = beta_by_sqrt3 << 1;

            const iq_t u = (1 + a + b) >> 1;
            const iq_t v = (1 - a + b) >> 1;
            const iq_t w = (1 - a - b) >> 1;

            return {u, v, w};
        }

        case Sector::_2:
        case Sector::_5:
        {
            const iq_t u = half + alpha;
            const iq_t v = half + beta_by_sqrt3;
            const iq_t w = half - beta_by_sqrt3;

            return {u, v, w};
        }

        case Sector::_3:
        case Sector::_6:
        {
            const iq_t a = beta_by_sqrt3 << 1;
            const iq_t b = - alpha - beta_by_sqrt3;

            const iq_t u = (1 - a - b) >> 1;
            const iq_t v = (1 + a + b) >> 1;
            const iq_t w = (1 - a + b) >> 1;

            return {u, v, w};
        }
    }
}

// void SVPWM3::setDuty(const real_t duty, const real_t rad) {
//     driver_ = SVM(iq_t(duty * cos(rad)), iq_t(duty * sin(rad)));
// }


void SVPWM3::setAbDuty(const real_t alaph, const real_t beta){
    driver_ = SVM(iq_t(alaph), iq_t(beta));
}

}