#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

namespace ymd::digipw{
class Spll{
public:
    __no_inline void coeff_calc();
public:

    struct Coeff{
        q16 osg_b0;
        q16 osg_b2;
        q16 osg_a1;
        q16 osg_a2;
        q16 osg_qb0;
        q16 osg_qb1;
        q16 osg_qb2;
        q16 b1;
        q16 b0;
    };

    struct Config{
        uint32_t fs;
        q16 ac_freq; 
        q16 b0_lpf ;
        q16 b1_lpf ;

        constexpr Coeff to_coeff() const {
            const auto fn = ac_freq;
            const auto delta_t_x256=(q16(256)/fs);

            const q16 delta_t = delta_t_x256 >> 8;
            const q16 wn= fn *q16(TAU);

            const q16 osgx_x256 = (q16)(wn*delta_t_x256);
            const q16 osgx = (q16)(wn*delta_t);
            
            const q16 osgy_x256 = (q16)(wn * delta_t_x256 * wn * delta_t);
            const q16 osgy = (q16)(wn * delta_t * wn * delta_t);

            const q16 temp = (q16)(256) / (osgx_x256 + osgy_x256 + 4 * 256);

            const auto osg_b0=((q16)osgx*temp);
            const auto osg_qb0=((q16)(q16(0.5f) * osgy) * temp);
            return Coeff{
                .osg_b0=((q16)osgx*temp),
                .osg_b2=((q16)(-1)*osg_b0),
                .osg_a1=((q16)(2 * (4-osgy))*temp),
                .osg_a2=((q16)(osgx-osgy-4)*temp),
                .osg_qb0=((q16)(q16(0.5f) * osgy) * temp),
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
        this->delta_t_x256=(q16(256)/cfg.fs);
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
        
        _theta_x256=0;
        
        sine=0;
        cosine=0;
    }

    constexpr void update(const q16 u0){
        // SPLL_1PH_SOGI_run(&spll1,(q16)u0);
        // Update the u[0] with the grid value
        u[0] = u0 >> volt_scale_bits;

        //
        // Orthogonal Signal Generator
        //
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

        _theta_x256 =_theta_x256 + (fo*(delta_t_x256*(q16)(TAU)));

        if(_theta_x256 >= (q16)(TAU * 256)){
            _theta_x256 -= (q16)(TAU * 256);
        }

        auto [sin_val, cos_val] = sincos(_theta_x256);
        sine = sin_val;
        cosine = cos_val;
    }

    q16 ferq() const{return this->fo;}

    q16 theta() const{return this->_theta_x256 >> 8;}

    q16 ud() const{return this->u_D[0] << volt_scale_bits;}

    q16 uq() const{return this->u_Q[0] << volt_scale_bits;}


private:
    // SPLL_1PH_SOGI spll1;

    static constexpr q16 default_ac_freq = 50.0_r;
    static constexpr q16 default_b0_lpf = 222.2862_r;
    static constexpr q16 default_b1_lpf = -222.034_r;
    static constexpr int volt_scale_bits = 3;
    // static constexpr int volt_scale_bits = 0;


    Coeff coeff_;
    q16   u[3] = {0,0,0};       // AC input data buffer
    q16   osg_u[3] = {0,0,0};   // Orthogonal signal generator data buffer
    q16   osg_qu[3] = {0,0,0};  // Orthogonal signal generator quadrature data buffer
    q16   u_Q[2] = {0,0};     // Q-axis component
    q16   u_D[2] = {0,0};     // D-axis component
    q16   ylf[2] = {0,0};     // Loop filter data storage
    q16   fn = 0;         // Nominal frequency (Hz)
    q16   delta_t_x256 = {0};    // Inverse of the ISR rate at which module is called
    q16   fo = {0};         // Output frequency of PLL(Hz)
    q16   _theta_x256 = {0};      // Angle output (0-2*pi)
    q16   cosine = {0};     // Cosine value of the PLL angle
    q16   sine = {0};       // Sine value of the PLL angle
};

}