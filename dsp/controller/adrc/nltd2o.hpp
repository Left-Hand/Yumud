#pragma once

#include "core/math/iq/fixed_t.hpp"
#include "core/utils/Result.hpp"
#include "dsp/state_vector.hpp"
#include "fhan.hpp"

namespace ymd::dsp{


struct NonlinearTrackingDifferentor{
    using fhan_type = FhanPrecomputed<iq16>;
    struct Coeffs{
        uq32 dt;
        fhan_type fhan;
        iq16 x2_limit;
    };

    struct Config{
        uint32_t fs;
        uq16 r;
        uq16 h;
        iq16 x2_limit;

        constexpr Result<Coeffs, const char *> try_to_coeffs() const {
            const auto & self = *this;
            return Ok(Coeffs{
                .dt = (std::numeric_limits<uq32>::max() / fs), 
                .fhan = (fhan_type(fhan_type::Config{.r = self.r, .h = self.h})),
                .x2_limit = self.x2_limit
            });
        }
    };

    constexpr explicit NonlinearTrackingDifferentor(const Coeffs & coeffs):
        coeffs_(coeffs)
        {;}
    

    constexpr SecondOrderState update(const SecondOrderState & state, const iq16 x1, const iq16 x2) const {
        const iq16 x1_now = iq16::from_bits(static_cast<int32_t>(state.x1.to_bits() >> 16));
        const iq16 x2_now = state.x2;
        
        const iq16 e1 = x1 - x1_now;
        const iq16 e2 = x2 - x2_now;
        const auto u = coeffs_.fhan(e1, e2);
        const auto next_x1 = state.x1 + extended_mul(state.x2, coeffs_.dt);
        const auto next_x2 = CLAMP2(state.x2 + u * coeffs_.dt, coeffs_.x2_limit);
        return {
            next_x1,
            next_x2
        };
    }

private:
    Coeffs coeffs_;
};

}