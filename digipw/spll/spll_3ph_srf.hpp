//#############################################################################
//
//  FILE:   splll_3ph_srf.h
//
//  TITLE:  Software Phase Lock Loop for Three Phase Grid Tied Systems
//
//#############################################################################
// $TI Release: Software Phase Lock Loop Library v1.03.00.00 $
// $Release Date: Tue Aug 26 14:08:30 CDT 2025 $
// $Copyright:
// Copyright (C) 2025 Texas Instruments Incorporated - http://www.ti.com/
//
// ALL RIGHTS RESERVED
// $
//#############################################################################
#pragma once

// https://dev.ti.com/tirex/explore/node?node=A__ALGTB-WevzhwhtRaHy71Iw__digital_power_c2000ware_sdk_software_package__5C6SEVO__LATEST

namespace ymd::digipw{

//
// Typedefs
//



// \brief          Defines the SPLL_3PH_SRF structure
//
// \details        This software module implements a software phase lock loop
//                 based on synchronous reference frame for
//                 grid connection to three phase grid
//
struct SPLL_3PH_SRF{
    struct Config{
        float grid_freq;
        float delta_t;
    };

    struct LPF_COEFF{
        float b1;
        float b0;
    } ;

    constexpr explicit SPLL_3PH_SRF(const Config & cfg){
        init(cfg.grid_freq, cfg.delta_t);
    }

    // \brief              Reset SPLL_3PH_SRF module
    // \param *spll_obj     The SPLL_3PH_SRF structure
    //
    constexpr void reset()
    {
        v_q[0] = (float)(0.0);
        v_q[1] = (float)(0.0);

        ylf[0] = (float)(0.0);
        ylf[1] = (float)(0.0);

        fo = (float)(0.0);

        theta[0] = (float)(0.0);
        theta[1] = (float)(0.0);

    }

    // \brief              Run SPLL_3PH_SRF module
    // \param v_q          Q component of the grid voltage
    // \param *spll_obj     The SPLL_3PH_SRF structure
    //
    constexpr void run(float u_q)
    {
        //
        // Update the v_q[0] with the grid value
        //
        v_q[0] = u_q;

        //
        // Loop Filter
        //
        ylf[0] =  ylf[1]
                        + (lpf_coeff.b0 * v_q[0])
                        + (lpf_coeff.b1 * v_q[1]);
        ylf[1] = ylf[0];
        v_q[1] = v_q[0];

        ylf[0] = (ylf[0] > (float)(200.0)) ?
                                    (float)(200.0) : ylf[0];

        //
        // VCO
        //
        fo = fn + ylf[0];

        theta[0] = theta[1] +
                            ((fo * delta_t) *
                            (float)(2.0 * 3.1415926));
        if(theta[0] > (float)(2.0 * 3.1415926))
        {
            theta[0] = theta[0] - (float)(2.0 * 3.1415926);
        }

        theta[1] = theta[0];
    }
private:
    float v_q[2];     //< Rotating reference frame Q-axis value
    float ylf[2];     //< Data buffer for loop filter output
    float fo;         //< Output frequency of PLL
    float fn;         //< Nominal frequency
    float theta[2];   //< Grid phase angle
    float delta_t;    //< Inverse of the ISR rate at which module is called
    LPF_COEFF lpf_coeff;  //< Loop filter coefficients

    // \brief              Initialize SPLL_3PH_SRF module
    // \param grid_freq    The grid frequency
    // \param delta_t      1/Frequency of calling the PLL routine
    // \param *spll_obj     The SPLL_3PH_SRF structure
    //
    constexpr void init(float grid_freq, float delta_t)
    {
        v_q[0] = (float)(0.0);
        v_q[1] = (float)(0.0);

        ylf[0] = (float)(0.0);
        ylf[1] = (float)(0.0);

        fo = (float)(0.0);
        fn = (float)(grid_freq);

        theta[0] = (float)(0.0);
        theta[1] = (float)(0.0);

        delta_t = (float)delta_t;
    }

};



}