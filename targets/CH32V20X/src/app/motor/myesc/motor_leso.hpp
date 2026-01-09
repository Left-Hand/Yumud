#pragma once


namespace ymd::dsp::adrc{



struct [[nodiscard]] MotorLeso{
public:

    struct [[nodiscard]] Coeffs{
        uq8 b0;
        uq32 dt;
        uq32 g1t;
        uq16 g2t;

        friend OutputStream & operator <<(OutputStream & os, const Coeffs & coeffs){
            return os << os.field("b0")(coeffs.b0) << os.splitter()
                << os.field("dt")(coeffs.dt) << os.splitter()
                << os.field("g1t")(coeffs.g1t) << os.splitter()
                << os.field("g2t")(coeffs.g2t);
        }
    };

    struct [[nodiscard]] Config{
        uint32_t fs;
        uint32_t fc;
        uq8 b0;

        constexpr Result<Coeffs, const char *> try_into_coeffs() const{
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
            return Ok(Coeffs{
                .b0 = self.b0,
                .dt = dt,
                .g1t = g1t,
                .g2t = g2t
            });
        }
    };

    using State = SecondOrderState<iq16>;

    constexpr explicit MotorLeso(const Coeffs & coeffs):
        coeffs_(coeffs){;}

    constexpr State iterate(const State & state, const iq16 y, const iq16 u) const {
        // dx1=x2+b0*u+g1*(y-x1);
        // dx2=g2*(y-x1);

        #if 0
        uq16 g1t = coeffs_.dt * coeffs_.g1;
        uq16 g2t = coeffs_.dt * coeffs_.g2;
        const auto e = (y - state.x1);
        const auto delta_x1 =  ((state.x2 + u * coeffs_.b0 ) * coeffs_.dt) + (e * g1t);
        const auto delta_x2 = (e * g2t);
        return State{
            state.x1 + delta_x1, 
            state.x2 + delta_x2
        };
        #else

        #if 1
        const auto e = (y - math::fixed_downcast<16>(state.x1));
        const auto delta_x1 = extended_mul((state.x2 + (u * coeffs_.b0)), coeffs_.dt)
            + extended_mul(e, coeffs_.g1t);
        const auto delta_x2 = (e * coeffs_.g2t);
        return State{
            state.x1 + delta_x1,
            state.x2 + delta_x2
        };
        #else
        const auto e = (y - state.x1);
        const auto delta_x1 = (state.x2 + u * coeffs_.b0 + e * coeffs_.g1) * coeffs_.dt;
        const auto delta_x2 = (e * coeffs_.g2) * coeffs_.dt;
        return State{
            state.x1 + delta_x1,
            state.x2 + delta_x2
        };
        #endif

        #endif
    }

private:
    using Self = MotorLeso;
    Coeffs coeffs_;
};

}