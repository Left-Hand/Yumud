#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/angle.hpp"

namespace ymd::digipw{
class Spll{
public:
    __no_inline void coeff_calc();
public:

    struct Coeff{
        iq16 osg_b0;
        iq16 osg_b2;
        iq16 osg_a1;
        iq16 osg_a2;
        iq16 osg_qb0;
        iq16 osg_qb1;
        iq16 osg_qb2;
        iq16 b1;
        iq16 b0;
    };

    struct Config{
        uint32_t fs;
        iq16 ac_freq; 
        iq16 b0_lpf ;
        iq16 b1_lpf ;

        constexpr Coeff to_coeff() const {
            const auto fn = ac_freq;
            const auto delta_t_x256=(iq16(256)/fs);

            const iq16 delta_t = delta_t_x256 >> 8;
            const iq16 wn= fn *iq16(TAU);

            const iq16 osgx_x256 = (iq16)(wn*delta_t_x256);
            const iq16 osgx = (iq16)(wn*delta_t);
            
            const iq16 osgy_x256 = (iq16)(wn * delta_t_x256 * wn * delta_t);
            const iq16 osgy = (iq16)(wn * delta_t * wn * delta_t);

            const iq16 temp = (iq16)(256) / (osgx_x256 + osgy_x256 + 4 * 256);

            const auto osg_b0=((iq16)osgx*temp);
            const auto osg_qb0=((iq16)(iq16(0.5f) * osgy) * temp);
            return Coeff{
                .osg_b0=((iq16)osgx*temp),
                .osg_b2=((iq16)(-1)*osg_b0),
                .osg_a1=((iq16)(2 * (4-osgy))*temp),
                .osg_a2=((iq16)(osgx-osgy-4)*temp),
                .osg_qb0=((iq16)(iq16(0.5f) * osgy) * temp),
                .osg_qb1=(osg_qb0 * 2),
                .osg_qb2=osg_qb0,
                .b1 = b1_lpf,
                .b0 = b0_lpf,

            };
        }
    };


    constexpr Spll(const Spll & other) = delete;
    constexpr Spll(Spll && other) = default;

    constexpr Spll(const Config & cfg):
        coeff_(cfg.to_coeff())
    {
        this->fn=cfg.ac_freq;
        this->dt_=(iq16(256)/cfg.fs);
        reset();
    }

    constexpr void reset(){
        u[0]=0;
        u[1]=0;
        u[2]=0;
        
        osg_u[0]=0;
        osg_u[1]=0;
        osg_u[2]=0;
        
        osg_qu[0]=0;
        osg_qu[1]=0;
        osg_qu[2]=0;
        
        u_Q[0]=0;
        u_Q[1]=0;
        
        u_D[0]=0;
        u_D[1]=0;
        
        ylf[0]=0;
        ylf[1]=0;
        
        fo=0;
        
        turns_=0;
        
        sine=0;
        cosine=0;
    }

    constexpr void update(const iq16 u0){
        // SPLL_1PH_SOGI_run(&spll1,(iq16)u0);
        // Update the u[0] with the grid value
        u[0] = u0;


        osg_u[0] = 
            (coeff_.osg_b0 * (u[0]-u[2])) +
            (coeff_.osg_a1 * osg_u[1]) +
            (coeff_.osg_a2 * osg_u[2]);

        osg_u[2] = osg_u[1];
        osg_u[1] = osg_u[0];

        osg_qu[0]=
            (coeff_.osg_qb0 * u[0]) +
            (coeff_.osg_qb1 * u[1]) +
            (coeff_.osg_qb2 * u[2]) +
            (coeff_.osg_a1 * osg_qu[1]) +
            (coeff_.osg_a2 * osg_qu[2]);

        osg_qu[2] = osg_qu[1];
        osg_qu[1] = osg_qu[0];

        u[2] = u[1];
        u[1] = u[0];

        u_Q[0] = 
            (cosine * osg_u[0]) +
            (sine * osg_qu[0]);

        u_D[0] = 
            (cosine * osg_qu[0]) -
            (sine * osg_u[0]);

        ylf[0] = 
            ylf[1] +
            (coeff_.b0 * u_Q[0]) +
            (coeff_.b1 * u_Q[1]);
        ylf[1] = ylf[0];

        u_Q[1] = u_Q[0];

        fo = fn + ylf[0];

        turns_ = frac(turns_ + (dt_ * fo));

        auto [sin_val, cos_val] = sincospu(turns_);
        sine = sin_val;
        cosine = cos_val;
    }

    iq16 freq() const{return this->fo;}

    Angle<iq16> angle() const{return Angle<iq16>::from_turns(static_cast<iq16>(turns_));}

    iq16 ud() const{return this->u_D[0];}

    iq16 uq() const{return this->u_Q[0];}


private:
    // SPLL_1PH_SOGI spll1;

    static constexpr iq16 default_ac_freq = 50.0_r;
    static constexpr iq16 default_b0_lpf = 222.2862_r;
    static constexpr iq16 default_b1_lpf = -222.034_r;
    // static constexpr int volt_scale_bits = 0;


    Coeff coeff_;
    iq16   u[3] = {0,0,0};       // AC input data buffer
    iq16   osg_u[3] = {0,0,0};   // Orthogonal signal generator data buffer
    iq16   osg_qu[3] = {0,0,0};  // Orthogonal signal generator quadrature data buffer
    iq16   u_Q[2] = {0,0};     // Q-axis component
    iq16   u_D[2] = {0,0};     // D-axis component
    iq16   ylf[2] = {0,0};     // Loop filter data storage
    iq16   fn = 0;         // Nominal frequency (Hz)
    iq24   dt_ = {0};    // Inverse of the ISR rate at which module is called
    iq16   fo = {0};         // Output frequency of PLL(Hz)
    iq24   turns_ = {0};      // turns output (0-1)
    iq16   cosine = {0};     // Cosine value of the PLL angle
    iq16   sine = {0};       // Sine value of the PLL angle
};

}