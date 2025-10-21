//#############################################################################
//
//  FILE:   spll_3ph_ddsrf.h
//
//  TITLE:  DDSRF PLL for Three Phase Grid Tied Systems
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

// https://dev.ti.com/tirex/explore/node?node=A__AK0MJHI91C6lJRUq2gW.QQ__digital_power_c2000ware_sdk_software_package__5C6SEVO__LATEST

namespace ymd::digipw{



// \brief          Defines the SPLL_3PH_DDSRF structure
//
// \details        This software module implements a software phase lock loop
//                 based on decoupled double synchronous reference frame for
//                 grid connection to three phase grid
//
typedef struct{
    typedef struct{
        float b1;
        float b0;
    } LPF_COEFF;

    struct Config{
        float grid_freq;
        float delta_t;
        float k1; 
        float k2;
    };

    // \brief              Initialize SPLL_3PH_DDSRF module
    // \param grid_freq    The grid frequency
    // \param delta_t      1/Frequency of calling the PLL routine
    // \param k1           parameter
    // \param k2           parameter
    // \param *spll_obj    The SPLL_3PH_DDSRF structure
    //
    constexpr void init(const Config & cfg)
    {
        d_p_decoupl = (float)(0.0);
        d_n_decoupl = (float)(0.0);

        q_p_decoupl = (float)(0.0);
        q_n_decoupl = (float)(0.0);

        d_p_decoupl_lpf = (float)(0.0);
        d_n_decoupl_lpf = (float)(0.0);

        q_p_decoupl_lpf = (float)(0.0);
        q_n_decoupl_lpf = (float)(0.0);

        y[0] = (float)(0.0);
        y[1] = (float)(0.0);

        x[0] = (float)(0.0);
        x[1] = (float)(0.0);

        w[0] = (float)(0.0);
        w[1] = (float)(0.0);

        z[0] = (float)(0.0);
        z[1] = (float)(0.0);

        k1 = cfg.k1;
        k2 = cfg.k2;

        v_q[0] = (float)(0.0);
        v_q[1] = (float)(0.0);

        ylf[0] = (float)(0.0);
        ylf[1] = (float)(0.0);

        fo = (float)(0.0);
        fn = (float)(cfg.grid_freq);

        theta[0] = (float)(0.0);
        theta[1] = (float)(0.0);

        delta_t = cfg.delta_t;
    }

    //
    // \brief              Reset SPLL_3PH_DDSRF module
    // \param *spll_obj    The SPLL_3PH_DDSRF structure
    //
    constexpr void reset()
    {
        d_p_decoupl = (float)(0.0);
        d_n_decoupl = (float)(0.0);

        q_p_decoupl = (float)(0.0);
        q_n_decoupl = (float)(0.0);

        d_p_decoupl_lpf = (float)(0.0);
        d_n_decoupl_lpf = (float)(0.0);

        q_p_decoupl_lpf = (float)(0.0);
        q_n_decoupl_lpf = (float)(0.0);

        y[0] = (float)(0.0);
        y[1] = (float)(0.0);

        x[0] = (float)(0.0);
        x[1] = (float)(0.0);

        w[0] = (float)(0.0);
        w[1] = (float)(0.0);

        z[0] = (float)(0.0);
        z[1] = (float)(0.0);

        v_q[0] = (float)(0.0);
        v_q[1] = (float)(0.0);

        ylf[0] = (float)(0.0);
        ylf[1] = (float)(0.0);

        fo = (float)(0.0);

        theta[0] = (float)(0.0);
        theta[1] = (float)(0.0);
    }

    //
    // \brief              Run spll_3PH_srf module
    // \param *spll_obj    The spll_3PH_ddsrf structure
    // \param d_p          D Positive seq component of the grid voltage
    // \param d_n          D Negative seq component of the grid voltage
    // \param q_p          Q Positive seq component of the grid voltage
    // \param q_n          Q Negative seq component of the grid voltage
    //
    constexpr void run(float d_p, float d_n,
                                        float q_p, float q_n)
    {
        //
        // before calling this routine run the ABC_DQ0_Pos & Neg run routines
        // pass updated values for d_p,d_n,q_p,q_n
        // and update the cos_2theta and sin_2theta values with the prev angle
        //

        //
        // Decoupling Network
        //
        d_p_decoupl = d_p
                            - (d_n_decoupl_lpf * cos_2theta)
                            - (q_n_decoupl * sin_2theta);
        q_p_decoupl = q_p
                            + (d_n_decoupl_lpf * sin_2theta)
                            - (q_n_decoupl * cos_2theta);

        d_n_decoupl = d_n
                            - (d_p_decoupl_lpf * cos_2theta)
                            + (q_p_decoupl * sin_2theta);
        q_n_decoupl = q_n
                            - (d_p_decoupl_lpf * sin_2theta)
                            - (q_p_decoupl * cos_2theta);

        //
        // Low pass filter
        //

        y[1] = (d_p_decoupl * k1)
                    - (y[0] * k2);
        d_p_decoupl_lpf = y[1] + y[0];
        y[0] = y[1];

        x[1] = (q_p_decoupl * k1)
                    - (x[0] * k2);
        q_p_decoupl_lpf = x[1] + x[0];
        x[0] = x[1];

        w[1] = (d_n_decoupl * k1)
                    - (w[0] * k2);
        d_n_decoupl_lpf = w[1] + w[0];
        w[0] = w[1];

        z[1] = (q_n_decoupl * k1)
                    - (z[0] * k2);
        q_n_decoupl_lpf = z[1] + z[0];
        z[0] = z[1];

        v_q[0] = q_p_decoupl;

        //
        // Loop Filter
        //
        ylf[0] = ylf[1]
                        + (lpf_coeff.b0 * v_q[0])
                        + (lpf_coeff.b1 * v_q[1]);
        ylf[1] = ylf[0];
        v_q[1] = v_q[0];

        //
        // VCO
        //
        fo = fn + ylf[0];

        theta[0] = theta[1] +
                ((fo * delta_t)
                * (float)(2.0f * 3.1415926f));

        if(theta[0] > (float)(2.0f * 3.1415926f))
        {
            theta[0] = theta[0] -
                    (float)(2.0f * 3.1415926f);
        }


        theta[1] = theta[0];

        cos_2theta = cosf(theta[1] * 2.0f);
        sin_2theta = sinf(theta[1] * 2.0f);
    }

private:
    float d_p_decoupl;  //< Positive Rotating reference Frame D-axis value
    float d_n_decoupl;  //< Negative Rotating reference Frame D-axis value
    float q_p_decoupl;  //< Positive Rotating reference Frame Q-axis value
    float q_n_decoupl;  //< Negative Rotating reference Frame Q-axis value

    float cos_2theta;   //< Cos of twice the grid frequency angle
    float sin_2theta;   //< Sin of twice the grid frequency angle

    float y[2];         //< Used to store history for filtering the decoupled D and Q axis components    
    float x[2];         //< Used to store history for filtering the decoupled D and Q axis components
    float w[2];         //< Used to store history for filtering the decoupled D and Q axis components
    float z[2];         //< Used to store history for filtering the decoupled D and Q axis components
    float k1;           //< Lpf coefficient
    float k2;           //< Lpf coefficient    
    float d_p_decoupl_lpf;  //< Decoupled positive sequence D-axis component filtered
    float d_n_decoupl_lpf;  //< Decoupled negative sequence D-axis component filtered
    float q_p_decoupl_lpf;  //< Decoupled positive sequence Q-axis component filtered
    float q_n_decoupl_lpf;  //< Decoupled negative sequence Q-axis component filtered

    float v_q[2];       
    float theta[2];     //< Grid phase angle
    float ylf[2];       //< Internal Data Buffer for Loop Filter output
    float fo;           //< Instantaneous Grid Frequency in Hz
    float fn;           //< Nominal Grid Frequency in Hz
    float delta_t;      //< 1/Frequency of calling the PLL routine
    LPF_COEFF lpf_coeff;
} SPLL_3PH_DDSRF;



}