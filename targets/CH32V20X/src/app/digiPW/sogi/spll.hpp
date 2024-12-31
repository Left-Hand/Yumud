#pragma once

#include "spll_c.hpp"

class Spll{
private:
    SPLL_1PH_SOGI spll1;

    scexpr real_t default_ac_freq = 50.0_r;
    scexpr real_t m_isr_freq = 25000.0_r;
    scexpr real_t m_b0_lpf = 222.2862_r;
    scexpr real_t m_b1_lpf = -222.034_r;

public:
    real_t ac_freq = 0;
    real_t isr_freq = 0;
    real_t b0_lpf = 0;
    real_t b1_lpf = 0;

    real_t u_d;
    real_t u_q;

    Spll(   real_t _ac_freq = default_ac_freq, 
            real_t _isr_freq = m_isr_freq, 
            real_t _b0_lpf = m_b0_lpf, 
            real_t _b1_lpf = m_b1_lpf):
        
        ac_freq(_ac_freq),isr_freq(_isr_freq),b0_lpf(_b0_lpf),b1_lpf(_b1_lpf){

            SPLL_1PH_SOGI_reset(&spll1);
            SPLL_1PH_SOGI_config(&spll1,
                    ac_freq,
                    isr_freq,
                    b0_lpf,
                    b1_lpf);
    }
    
    real_t update(const real_t u0){
        SPLL_1PH_SOGI_run(&spll1,(real_t)u0);
        u_d = spll1.u_D[0];
        u_q = spll1.u_Q[0];
        return spll1.sine;
    }

    real_t get_ferq() const{
        return spll1.fo;
    }

    real_t get_theta() const{
        return spll1.theta;
    }

    ~Spll(){;}
};