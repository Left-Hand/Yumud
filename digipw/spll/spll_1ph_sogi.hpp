//#############################################################################
//
//  FILE:   spll_1ph_sogi.h
//
//  TITLE:  Orthogonal Signal Generator Software Phase Lock Loop (SPLL) 
//          for Single Phase Grid Module
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

#include "core/math/realmath.hpp"

// https://dev.ti.com/tirex/explore/node?node=A__ANOOJs3tI6p9wcttAq8vKw__digital_power_c2000ware_sdk_software_package__5C6SEVO__LATEST

namespace ymd::digipw::spll{


struct SPLL_1PH_SOGI{
    struct OsgCoeff{
        float osg_k;
        float osg_x;
        float osg_y;
        float osg_b0;
        float osg_b2;
        float osg_a1;
        float osg_a2;
        float osg_qb0;
        float osg_qb1;
        float osg_qb2;
    } ;

    typedef struct{
        float b1;
        float b0;
    } LpfCoeff;

    struct Config{
        float acFreq;
        float isrFrequency;
        float lpf_b0;
        float lpf_b1;

        [[nodiscard]] constexpr OsgCoeff to_coeff() const {
            const float fn=acFreq;
            const float delta_t=((1.0f)/isrFrequency);
            OsgCoeff osg_coeff;
            float osgx,osgy,temp, wn;
            wn= fn *(float) 2.0f * (float) 3.14159265f;
            osg_coeff.osg_k=(float)(0.5);
            osgx = (float)(2.0f*0.5f*wn*delta_t);
            osg_coeff.osg_x=(float)(osgx);
            osgy = (float)(wn*delta_t*wn*delta_t);
            osg_coeff.osg_y=(float)(osgy);
            temp = (float)1.0/(osgx+osgy+4.0f);
            osg_coeff.osg_b0=((float)osgx*temp);
            osg_coeff.osg_b2=((float)(-1.0f)*osg_coeff.osg_b0);
            osg_coeff.osg_a1=((float)(2.0*(4.0f-osgy))*temp);
            osg_coeff.osg_a2=((float)(osgx-osgy-4)*temp);
            osg_coeff.osg_qb0=((float)(0.5f*osgy)*temp);
            osg_coeff.osg_qb1=(osg_coeff.osg_qb0*(float)(2.0));
            osg_coeff.osg_qb2=osg_coeff.osg_qb0;

            return osg_coeff;
        }
    };


    explicit constexpr SPLL_1PH_SOGI(const Config & cfg):
        osg_coeff(cfg.to_coeff()),
        lpf_coeff(LpfCoeff{cfg.lpf_b0, cfg.lpf_b1}){

        }
    // \brief Resets internal storage data of the module
    // \param *spll_obj The SPLL_1PH_SOGI structure pointer
    //

    // \brief Calculates the SPLL_1PH_SOGI coefficient
    // \param *spll_obj The SPLL_1PH_SOGI structure
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
    }



    // \brief Run the SPLL_1PH_SOGI module
    // \param *spll_obj The SPLL_1PH_SOGI structure pointer
    // \param acValue AC grid voltage in per unit (pu)
    //
    constexpr void run(float acValue){
        // Update the u[0] with the grid value
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

        u_Q[1]=u_Q[0];

        //
        // VCO
        //
        fo=fn+ylf[0];

        theta=theta + (fo*delta_t)*
                        (float)(2.0*3.1415926f);

        if(theta>(float)(2.0*3.1415926f))
        {
            theta=theta - (float)(2.0*3.1415926f);
            //theta=0;
        }


        auto [s, c] = sincos(theta);
        sine = s;
        cosine = c;
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
    float   theta;      //< Angle output (0-2*pi)
    float   cosine;     //< Cosine value of the PLL angle
    float   sine;       //< Sine value of the PLL angle
    float   delta_t;    //< Inverse of the ISR rate at which module is called
    OsgCoeff osg_coeff; //< Orthogonal signal generator coefficient
    LpfCoeff lpf_coeff; //< Loop filter coeffcient structure
};

}