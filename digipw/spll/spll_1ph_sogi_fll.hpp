//#############################################################################
//
// FILE:      spll_1ph_sogi_fll.h
//
// TITLE:     Orthogonal Signal Generator Software Phase Lock Loop (SPLL) for 
//            Single Phase Grid with Frequency Locked Loop (FLL) Module
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

namespace ymd::digipw{

// \brief Defines the Orthogonal Signal Generator SPLL_1PH_SOGI_FLL structure
//
//
// \details The SPLL_1PH_SOGI_FLL can be used to generate the
//          orthogonal signal from the sensed single phase grid voltage
//          and use that information to provide phase of the grid voltage
//          This module also runs a frequency locked loop for adjusting the
//          the center frequency automatically
//
typedef struct{

    typedef struct{
        float osg_b0;
        float osg_b2;
        float osg_a1;
        float osg_a2;
        float osg_qb0;
        float osg_qb1;
        float osg_qb2;
    } OSG_COEFF;

    // \brief Defines the LPF_COEFF structure
    //
    typedef struct{
        float b1;
        float b0;
    } LPF_COEFF;

    // \brief Resets internal data to zero,
    // \param *spll_obj  The SPLL_1PH_SOGI_FLL structure pointer
    //
    constexpr void reset()
    {
        u[0]=(float)(0.0);
        u[1]=(float)(0.0);
        u[2]=(float)(0.0);
        
        osg_u[0]=(float)(0.0);
        osg_u[1]=(float)(0.0);
        osg_u[2]=(float)(0.0);
        
        osg_qu[0]=(float)(0.0);
        osg_qu[1]=(float)(0.0);
        osg_qu[2]=(float)(0.0);
        
        u_Q[0]=(float)(0.0);
        u_Q[1]=(float)(0.0);
        
        u_D[0]=(float)(0.0);
        u_D[1]=(float)(0.0);
        
        ylf[0]=(float)(0.0);
        ylf[1]=(float)(0.0);
        
        fo=(float)(0.0);
        
        theta=(float)(0.0);
        
        sine=(float)(0.0);
        cosine=(float)(0.0);

        x3[0]=0;
        x3[1]=0;

        ef2=0;
    }

    // \brief Calculates the SPLL_1PH_SOGI_FLL coefficients
    // \param *spll_obj The SPLL_1PH_SOGI_FLL structure pointer
    //
    constexpr void coeff_calc()
    {
        float osgx,osgy,temp;

        osgx = (float)(2.0f*k*w_dash*delta_t);
        osgy = (float)(w_dash*delta_t*w_dash*delta_t);
        temp = (float)1.0f/(osgx+osgy+4.0f);

        osg_coeff.osg_b0=((float)osgx*temp);
        osg_coeff.osg_b2=((float)(-1.0f)*osg_coeff.osg_b0);
        osg_coeff.osg_a1=((float)(2.0f*(4.0f-osgy))*temp);
        osg_coeff.osg_a2=((float)(osgx-osgy-4)*temp);

        osg_coeff.osg_qb0=((float)(k*osgy)*temp);
        osg_coeff.osg_qb1=(osg_coeff.osg_qb0*(float)(2.0));
        osg_coeff.osg_qb2=osg_coeff.osg_qb0;

        x3[0]=0;
        x3[1]=0;
    }

    // \brief  Configures the SPLL_1PH_SOGI_FLL coefficients
    // \param  *spll_obj The SPLL_1PH_SOGI_FLL structure pointer
    // \param  acFreq Nominal AC frequency for the SPLL Module
    // \param  isrFrequency Nominal AC frequency for the SPLL Module
    // \param  lpf_b0 B0 coefficient of LPF of SPLL
    // \param  lpf_b1 B1 coefficient of LPF of SPLL
    // \param  k k parameter for FLL
    // \param  gamma gamma parameter for FLL
    //
    constexpr void config(
                            float acFreq,
                            float isrFrequency,
                            float lpf_b0,
                            float lpf_b1,
                            float k,
                            float gamma)
    {
        fn=acFreq;
        w_dash = 2*3.14159265f*acFreq;
        wc = 2*3.14159265f*acFreq;
        delta_t=((1.0f)/isrFrequency);
        k=k;
        gamma=gamma;

        coeff_calc();

        lpf_coeff.b0=lpf_b0;
        lpf_coeff.b1=lpf_b1;
    }

    // \brief  Runs SPLL_1PH_SOGI_FLL module
    // \param  *spll_obj The SPLL_1PH_SOGI_FLL structure pointer
    // \param  acValue AC grid voltage in per unit (pu)
    //
    constexpr void run(float acValue)
    {
        float osgx,osgy,temp;

        //
        // Update the u[0] with the grid value
        //
        u[0]=acValue;

        //
        // Orthogonal Signal Generator
        //
        osg_u[0]=(osg_coeff.osg_b0*
                        (u[0]-u[2])) +
                        (osg_coeff.osg_a1*osg_u[1]) +
                        (osg_coeff.osg_a2*osg_u[2]);

        osg_u[2]=osg_u[1];
        osg_u[1]=osg_u[0];

        osg_qu[0]=(osg_coeff.osg_qb0*u[0]) +
                            (osg_coeff.osg_qb1*u[1]) +
                            (osg_coeff.osg_qb2*u[2]) +
                            (osg_coeff.osg_a1*osg_qu[1]) +
                            (osg_coeff.osg_a2*osg_qu[2]);

        osg_qu[2]=osg_qu[1];
        osg_qu[1]=osg_qu[0];

        u[2]=u[1];
        u[1]=u[0];

        //
        // Park Transform from alpha beta to d-q axis
        //
        u_Q[0]=(cosine*osg_u[0]) +
                        (sine*osg_qu[0]);
        u_D[0]=(cosine*osg_qu[0]) -
                        (sine*osg_u[0]);

        //
        // Loop Filter
        //
        ylf[0]=ylf[1] +
                        (lpf_coeff.b0*u_Q[0]) +
                        (lpf_coeff.b1*u_Q[1]);
        ylf[1]=ylf[0];

        //ylf[0] = (ylf[0]>0.5)?0.5:ylf[0];
        //ylf[0] = (ylf[0]<-0.5)?-0.5:ylf[0];

        u_Q[1]=u_Q[0];

        //
        // VCO
        //
        fo=fn+ylf[0];

        theta=theta + (fo*delta_t)*
                        (float)(2.0*3.1415926f);

        if(theta>(float)(2.0*3.1415926f))
        {
            theta=theta-(float)(2.0*3.1415926f);
        }

        sine=(float)sinf(theta);
        cosine=(float)cosf(theta);

        //
        // FLL
        //
        ef2 = ((u[0] - osg_u[0])*osg_qu[0])
                        * gamma * delta_t*-1.0f;

        x3[0]=x3[1] + ef2;

        //x3[0]= (x3[0]>1.0)?1.0:x3[0];
        //x3[0]= (x3[0]<-1.0)?-1.0:x3[0];

        x3[1]=x3[0];

        w_dash = wc + x3[0];

        fn = w_dash / (2.0*3.1415926f);

        osgx = (float)(2.0f*k*w_dash*delta_t);
        osgy = (float)(w_dash * delta_t * w_dash *
                    delta_t);
        temp = (float)1.0f/(osgx+osgy+4.0f);

        osg_coeff.osg_b0=((float)osgx*temp);
        osg_coeff.osg_b2=((float)(-1.0f)*osg_coeff.osg_b0);
        osg_coeff.osg_a1=((float)(2.0f*(4.0f-osgy))*temp);
        osg_coeff.osg_a2=((float)(osgx-osgy-4)*temp);

        osg_coeff.osg_qb0=((float)(k*osgy)*temp);
        osg_coeff.osg_qb1=(osg_coeff.osg_qb0*(float)(2.0));
        osg_coeff.osg_qb2=osg_coeff.osg_qb0;
    }

private:
    float   u[3];       //< AC input data buffer
    float   osg_u[3];   //< Orthogonal signal generator data buffer
    float   osg_qu[3];  //< Orthogonal signal generator quadrature data buffer
    float   u_Q[2];     //< Q-axis component
    float   u_D[2];     //< D-axis component
    float   ylf[2];     //< Loop filter data storage
    float   fo;         //< Output frequency of PLL(Hz)
    float   fn;         //< Nominal frequency (Hz)
    float   wc;         //< Center (Nominal) frequency in radians
    float   theta;      //< Angle output (0-2*pi)
    float   cosine;     //< Cosine value of the PLL angle
    float   sine;       //< Sine value of the PLL angle
    float   delta_t;    //< Inverse of the ISR rate at which module is called
    float   ef2;        //< FLL parameter
    float   x3[2];      //< FLL data storage
    float   w_dash;     //< Output frequency of PLL(radians)
    float   gamma;      //< Gamma parameter for FLL
    float   k;          //< K parameter for FLL
    OSG_COEFF osg_coeff; //< Orthogonal signal generator coefficient
    LPF_COEFF lpf_coeff; //< Loop filter coeffcient structure
} SPLL_1PH_SOGI_FLL;


}