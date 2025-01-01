#pragma once

#include "sys/math/real.hpp"

namespace ymd{
class Spll{
public:

private:
    // SPLL_1PH_SOGI spll1;

    scexpr real_t default_ac_freq = 50.0_r;
    scexpr real_t default_b0_lpf = 222.2862_r;
    scexpr real_t default_b1_lpf = -222.034_r;
    scexpr int volt_scale_bits = 3;
    // scexpr int volt_scale_bits = 0;
    struct Coeff{
        real_t osg_b0;
        real_t osg_b2;
        real_t osg_a1;
        real_t osg_a2;
        real_t osg_qb0;
        real_t osg_qb1;
        real_t osg_qb2;
        real_t b1;
        real_t b0;
    };

    Coeff coeff;
    real_t   u[3];       //!< AC input data buffer
    real_t   osg_u[3];   //!< Orthogonal signal generator data buffer
    real_t   osg_qu[3];  //!< Orthogonal signal generator quadrature data buffer
    real_t   u_Q[2];     //!< Q-axis component
    real_t   u_D[2];     //!< D-axis component
    real_t   ylf[2];     //!< Loop filter data storage
    real_t   fn;         //!< Nominal frequency (Hz)
    real_t   delta_t_x256;    //!< Inverse of the ISR rate at which module is called
    real_t   fo;         //!< Output frequency of PLL(Hz)
    real_t   _theta;      //!< Angle output (0-2*pi)
    real_t   cosine;     //!< Cosine value of the PLL angle
    real_t   sine;       //!< Sine value of the PLL angle

    __no_inline void coeff_calc();
public:

    Spll(const Spll & other) = delete;
    Spll(Spll && other) = default;

    Spll(   uint _isr_freq,
            real_t _ac_freq = default_ac_freq, 
            real_t _b0_lpf = default_b0_lpf, 
            real_t _b1_lpf = default_b1_lpf){

            reset();

            this->fn=_ac_freq;
            this->delta_t_x256=(real_t(256)/_isr_freq);

            coeff_calc();

            this->coeff.b0 = _b0_lpf;
            this->coeff.b1 = _b1_lpf;
    }

    void reset();

    void update(const real_t u0);

    real_t ferq() const{return this->fo;}

    real_t theta() const{return this->_theta;}

    real_t ud() const{return this->u_D[0] << volt_scale_bits;}

    real_t uq() const{return this->u_Q[0] << volt_scale_bits;}

};

}