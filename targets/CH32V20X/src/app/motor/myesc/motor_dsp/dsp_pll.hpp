#pragma once


#include "core/math/fixed/fxmath.hpp"
#include "core/utils/Result.hpp"
#include "core/string/view/string_view.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "dsp_vec.hpp"


namespace ymd::dsp{

struct [[nodiscard]] alignas(4) PllState final{
    Angular<iq16> angluar_speed_integral;
    Angular<iq16> angluar_speed;
    Angular<uq32> angle;
    iq31 sine;
    iq31 cosine;

    void reset(){
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wclass-memaccess"
        memset(this, 0, sizeof(*this));
        #pragma GCC diagnostic pop
    }
};

struct [[nodiscard]] alignas(4) PllCoeffs final{
    size_t kp;
    uq16 ki_discrete;
    uq32 ts;

    constexpr void iterate(PllState & state, const std::array<iq16, 2> normalized_sincos) const {
        iq16 err = dsp::cross2v2(
            normalized_sincos[0], state.sine, 
            normalized_sincos[1], state.cosine
        );

        iterate_err(state, err);
    }

    constexpr void iterate_err(PllState & state, iq16 normalized_err) const {
        auto & self = *this;

        normalized_err = CLAMP2(normalized_err, 1.00_iq16);

        const auto angluar_speed = make_angular_from_turns(normalized_err * self.kp + state.angluar_speed_integral.to_turns());
        uint32_t angle_bits = state.angle.to_turns().to_bits();
        
        angle_bits += static_cast<uint32_t>((
            static_cast<uint64_t>(ts.to_bits()) * (angluar_speed.to_turns()).to_bits()
        ) >> 16);

        state.angle = state.angle.from_turns(
            uq32::from_bits(angle_bits)
        );

        std::tie(state.sine, state.cosine) = state.angle.sincos();
        state.angluar_speed = angluar_speed;
        state.angluar_speed_integral = state.angluar_speed_integral.from_turns(
            state.angluar_speed_integral.to_turns() + normalized_err * self.ki_discrete);
    }

    static constexpr PllCoeffs from_fskpki(const size_t fs, const size_t kp, const size_t ki){
        const uq16 ki_discrete = uq16::from_bits(ki) / uq16::from_bits(fs);
        const uq32 ts = uq32::from_rcp(fs);

        return PllCoeffs{
            .kp = kp,
            .ki_discrete = ki_discrete,
            .ts = ts
        };
    }
    static constexpr PllCoeffs from_fsfc(const size_t fs, const size_t fc, const uq8 zeta = 1){
        const size_t kp = size_t(zeta * 2 * fc);
        const size_t ki = fc * fc;
        return from_fskpki(fs, kp, ki);
    }
};

}