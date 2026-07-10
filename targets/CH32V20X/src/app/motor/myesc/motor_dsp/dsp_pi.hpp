#pragma once


#include "core/math/fixed/fxmath.hpp"
#include "core/utils/Result.hpp"
#include "core/string/view/string_view.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "dsp_vec.hpp"


namespace ymd::dsp{

struct [[nodiscard]] PiCofficients { 
    iq20 kp;                // 比例系数
    iq20 ki_discrete;       // 离散化积分系数（Ki * Ts）

    friend OutputStream & operator << (OutputStream & os, const PiCofficients & self){
        return os << self.kp << 
            os.splitter() << self.ki_discrete 
        ;
    }
};

struct LrSeriesCurrentRegulatorConfig{
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 截止频率/带宽 (Hz)
    iq20 phase_inductance_mh;        // 相电感 (mH)
    iq20 phase_resistance_ohm;        // 相电阻 (Ω)

    [[nodiscard]] constexpr Result<PiCofficients, StringView> 
    try_into_precomputed() const noexcept {
        //U(s) = I(s) * R + s * I(s) * L
        //I(s) / U(s) = 1 / (R + sL)
        //G_open(s) = (Ki / s + Kp) / s(R / s + L)

        // Ki = 2pi * fc * R
        // Kp = 2pi * fc * L

        if(fs >= 65535) return Err(StringView("fs too large"));
        if(fc * 8 >= fs) return Err(StringView("fc too large"));

        const auto & self = *this;
        PiCofficients coeffs;

        //norm_omega = fc * 2pi / fs

        const auto factor = (uq16::from_bits(TAU_SCALE_NUM * fc)
            / uq16::from_bits(TAU_SCALE_DEN * 1000));

        const auto norm_omega = uq32::from_bits(TAU_SCALE_NUM * fc) 
            / uq32::from_bits(TAU_SCALE_DEN * self.fs);

        coeffs.kp = self.phase_inductance_mh * factor;
        coeffs.ki_discrete = self.phase_resistance_ohm * norm_omega;                                    
        return Ok(coeffs);
    }
};



}