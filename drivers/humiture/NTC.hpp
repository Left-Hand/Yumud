#pragma once

#include "core/math/realmath.hpp"

namespace ymd::drivers{

class [[nodiscard]] NtcCalculator final{
public:
    struct Config{
        uint32_t B0;
        iq16 r0_kohms;
        iq16 pull_r_kohms;
    };

    constexpr NtcCalculator(const Config & cfg):
        cfg_(cfg){;}

    constexpr iq16 operator()(const iq16 normed_voltage){
        constexpr iq16 T0= iq16(273.15+25);
        constexpr iq16 Ka= iq16(273.15);
        iq16 vr = normed_voltage;
        iq16 rt_kohms = (vr)/(1-vr) * cfg_.pull_r_kohms;
        return iq16(iq16(cfg_.B0)/(iq16(cfg_.B0)/T0+math::ln(rt_kohms/cfg_.r0_kohms))) - Ka + iq16(0.5);
    }
private:
    const Config cfg_;
};



};