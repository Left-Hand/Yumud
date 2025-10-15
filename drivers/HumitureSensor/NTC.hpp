#pragma once

#include "core/math/realmath.hpp"

namespace ymd::drivers{

class [[nodiscard]] NtcCalculator final{
public:
    struct Config{
        uint32_t B0;
        q16 r0_kohms;
        q16 pull_r_kohms;
    };

    constexpr NtcCalculator(const Config & cfg):
        cfg_(cfg){;}

    constexpr q16 operator()(const q16 normed_voltage){
        static constexpr q16 T0= q16(273.15+25);
        static constexpr q16 Ka= q16(273.15);
        q16 VR = normed_voltage;
        q16 Rt_kOhms = (VR)/(1-VR) * cfg_.pull_r_kohms;
        return q16(q16(cfg_.B)/(q16(cfg_.B)/T0+log(Rt_kOhms/cfg_.r0_kohms))) - Ka + q16(0.5);
    }
private:
    const Config cfg_;
};



};