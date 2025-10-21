//#############################################################################
//
//  FILE      spll_1ph_notch.h
//
//  TITLE     Notch Filter based Software Phase Lock Loop (SPLL) 
//            for Single Phase Grid Module
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

// https://dev.ti.com/tirex/explore/node?node=A__AHOkM9zD3UrbPXWR8LPLDA__digital_power_c2000ware_sdk_software_package__5C6SEVO__LATEST

namespace ymd::digipw{

// \brief Defines the SPLL_1PH_NOTCH
//        structure
//
// \details The SPLL_1PH_NOTCH can be used to extract the phase of
//          the ac grid from the sensed single phase grid voltage
//
typedef struct{
    typedef struct{
        float b2;
        float b1;
        float b0;
        float a2;
        float a1;
    } COEFF;

    // \brief Defines the LPF_COEFF structure
    //
    typedef struct{
        float b1;
        float b0;
        float a1;
    } LPF_COEFF;
    constexpr void reset()
    {
        upd[0]=(float)(0.0);
        upd[1]=(float)(0.0);
        
        y_notch1[0]=(float)(0.0);
        y_notch1[1]=(float)(0.0);
        y_notch1[2]=(float)(0.0);
        
        y_notch2[0]=(float)(0.0);
        y_notch2[1]=(float)(0.0);
        y_notch2[2]=(float)(0.0);
        
        ylf[0]=(float)(0.0);
        ylf[1]=(float)(0.0);
        
        fo=(float)(0.0);
        
        theta=(float)(0.0);

        sine=(float)(0.0);
        cosine=(float)(0.0);
    }

    // \brief Calculates the coefficients for SPLL_1PH_NOTCH filter
    // \param *spll_obj The SPLL_1PH_NOTCH structure pointer
    // \param c1 c1 Notch paramater
    // \param c2 c2 Notch Parameter
    //
    constexpr void coeff_calc(float c1, float c2)
    {
        float notch_freq;
        float temp1,temp2;
        float wn2;
        float Ts, Fs;

        notch_freq=2*3.14159265f*fn;
        Ts = delta_t;
        Fs=1/Ts;

        //
        // pre warp the notch frequency
        //
        wn2=2*Fs*tanf(notch_freq* ((float)3.141592653589)*Ts);

        temp1= 4*Fs*Fs + 4* wn2 * c2 * Fs + wn2*wn2;
        temp2= 1/ ( 4*Fs*Fs + 4* wn2 * c1 * Fs + wn2*wn2);

        notch_coeff.b0 = temp1* temp2;
        notch_coeff.b1 = (-8*Fs*Fs + 2* wn2* wn2)* temp2;
        notch_coeff.b2 = (4*Fs*Fs-4*wn2*c2*Fs+wn2*wn2)*temp2;
        notch_coeff.a1 = (-8*Fs*Fs + 2* wn2* wn2)*temp2;
        notch_coeff.a2 = (4*Fs*Fs-4*wn2*c1*Fs+wn2*wn2)*temp2;
    }


    // \brief  Configures the SPLL_1PH_NOTCH module
    // \param  *spll_obj The SPLL_1PH_NOTCH structure pointer
    // \param  acFreq Nominal AC frequency for the SPLL Module
    // \param  isrFrequency Nominal AC frequency for the SPLL Module
    // \param  lpf_b0 B0 coefficient of LPF of SPLL
    // \param  lpf_b1 B1 coefficient of LPF of SPLL
    // \param  c1 c1 Notch paramater
    // \param  c2 c2 Notch Parameter
    //
    constexpr void config(SPLL_1PH_NOTCH *spll_obj,
                            float acFreq,
                            float isrFrequency,
                            float lpf_b0,
                            float lpf_b1,
                            float c1,
                            float c2
                            )
    {
        fn=acFreq;
        delta_t=((1.0f)/isrFrequency);

        coeff_calc(c1,c2);

        lpf_coeff.b0=lpf_b0;
        lpf_coeff.b1=lpf_b1;
    }

    // \brief  Runs the SPLL_1PH_NOTCH module
    // \param  *spll_obj The SPLL_1PH_NOTCH structure pointer
    // \param  acValue AC grid voltage in per unit (pu)
    //
    constexpr void run(float acValue)
    {
        //
        // Phase detect
        //
        upd[0] = acValue*cosine;
        
        //
        // Notch Filter
        //
        y_notch1[0] = - y_notch1[1]*notch_coeff.a1
                                - y_notch1[2]*notch_coeff.a2
                                + upd[0]*notch_coeff.b0
                                + upd[1]*notch_coeff.b1
                                + upd[2]*notch_coeff.b2;

        y_notch2[0] = - y_notch2[1]*notch_coeff.a1
                                - y_notch2[2]*notch_coeff.a2
                                + y_notch1[0]*notch_coeff.b0
                                + y_notch1[1]*notch_coeff.b1
                                + y_notch1[2]*notch_coeff.b2;

        //
        // Loop Filter
        //
        ylf[0] = ylf[1]
                        + (lpf_coeff.b0*y_notch2[0])
                        + (lpf_coeff.b1*y_notch2[1]);

        //ylf[0] = (ylf[0]>100)?100:ylf[0];
        //ylf[0] = (ylf[0]<-100)?-100:ylf[0];

        //
        // update the Upd array for future sample
        //
        upd[2] = upd[1];
        upd[1] = upd[0];

        y_notch1[2] = y_notch1[1];
        y_notch1[1] = y_notch1[0];

        y_notch2[2] = y_notch2[1];
        y_notch2[1] = y_notch2[0];

        ylf[1] = ylf[0];

        //
        // VCO
        //
        fo = fn+ylf[0];

        theta= theta
            + (fo*delta_t)*(float)(2.0*3.1415926);

        if(theta>(float)(2.0*3.1415926))
        {
            theta =theta-(float)(2.0*3.1415926);
        }

        sine = (float)sinf(theta);
        cosine = (float)cosf(theta);
    }
private:
    float   upd[3];       //< Phase detect buffer
    float   y_notch1[3];  //< Notch filter1 data storage
    float   y_notch2[3];  //< Notch filter2 data storage
    float   ylf[2];       //< Loop filter data storage
    float   fo;           //< Output frequency of PLL(Hz)
    float   fn;           //< Nominal frequency (Hz)
    float   theta;        //< Angle output (0-2*pi)
    float   cosine;       //< Cosine value of the PLL angle
    float   sine;         //< Sine value of the PLL angle
    float   delta_t;      //< Inverse of the ISR rate at which module is called
    COEFF notch_coeff; //< Notch filter coeffcient structure
    LPF_COEFF lpf_coeff;     //< Loop filter coeffcient structure
} SPLL_1PH_NOTCH;



}