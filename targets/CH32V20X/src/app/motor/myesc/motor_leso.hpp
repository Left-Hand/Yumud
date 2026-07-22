#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::dsp::adrc{



struct [[nodiscard]] MotorLeso{
public:
    using Self = MotorLeso;

    uq8 b0;
    uq32 dt;
    uq32 g1t;
    uq16 g2t;


    struct [[nodiscard]] Config{
        uint32_t fs;
        uint32_t fc;
        uq8 b0;

        constexpr Result<Self, const char *> try_into_precomputed() const noexcept {
            auto & self = *this;
            const auto dt = uq32::from_rcp(self.fs);
            if(self.fs >= 65536) 
                return Err("fs too large");
            if(self.fc * 2 >= fs ) 
                return Err("fc too large");

            const uq32 g1t = uq32::from_bits(static_cast<uint32_t>(
                2u * uint64_t(fc) * uint64_t(uint64_t(1) << 32) / fs));
            const uq16 g2t = uq16::from_bits(static_cast<uint32_t>(
                uint64_t(fc)  * uint64_t(fc) * uint64_t(uint64_t(1) << 16) / fs));
            return Ok(Self{
                .b0 = self.b0,
                .dt = dt,
                .g1t = g1t,
                .g2t = g2t
            });
        }
    };

    using State = SecondOrderState<iq16>;


    constexpr void iterate(State & state, const iq16 y, const iq16 u) const noexcept {
        auto & self = *this;

        // dx1=x2+b0*u+g1*(y-x1);
        // dx2=g2*(y-x1);

        const auto e = (y - math::fixed_downcast<16>(state.x1));
        const auto delta_x1 = extended_mul((state.x2 + (u * self.b0)), self.dt)
            + extended_mul(e, self.g1t);
        const auto delta_x2 = (e * self.g2t);
        
        state.x1 += delta_x1;
        state.x2 += delta_x2;

    }

    friend OutputStream & operator <<(OutputStream & os, const Self & coeffs){
        return os << os.field("b0")(coeffs.b0) << os.splitter()
            << os.field("dt")(coeffs.dt) << os.splitter()
            << os.field("g1t")(coeffs.g1t) << os.splitter()
            << os.field("g2t")(coeffs.g2t);
    }
};

}