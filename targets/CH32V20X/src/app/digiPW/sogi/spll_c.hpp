#pragma once

//*********** Structure Init Function ****//
#include "sys/math/real.hpp"


//
// Typedefs
//

//! \brief  Defines the SPLL_1PH_SOGI_OSG_COEFF structure
//!
typedef struct{
    real_t osg_k;
    real_t osg_x;
    real_t osg_y;
    real_t osg_b0;
    real_t osg_b2;
    real_t osg_a1;
    real_t osg_a2;
    real_t osg_qb0;
    real_t osg_qb1;
    real_t osg_qb2;
} SPLL_1PH_SOGI_OSG_COEFF;

//! \brief  Defines the SPLL_1PH_SOGI_LPF_COEFF structure
//!
typedef struct{
    real_t b1;
    real_t b0;
} SPLL_1PH_SOGI_LPF_COEFF;

//! \brief Defines the Orthogonal Signal Generator SPLL_1PH_SOGI
//!        structure
//!
//! \details The SPLL_1PH_SOGI can be used to generate the
//!          orthogonal signal from the sensed single phase grid voltage
//!          and use that information to provide phase of the grid voltage
//!
typedef struct{
    real_t   u[3];       //!< AC input data buffer
    real_t   osg_u[3];   //!< Orthogonal signal generator data buffer
    real_t   osg_qu[3];  //!< Orthogonal signal generator quadrature data buffer
    real_t   u_Q[2];     //!< Q-axis component
    real_t   u_D[2];     //!< D-axis component
    real_t   ylf[2];     //!< Loop filter data storage
    real_t   fo;         //!< Output frequency of PLL(Hz)
    real_t   fn;         //!< Nominal frequency (Hz)
    real_t   theta;      //!< Angle output (0-2*pi)
    real_t   cosine;     //!< Cosine value of the PLL angle
    real_t   sine;       //!< Sine value of the PLL angle
    real_t   delta_t;    //!< Inverse of the ISR rate at which module is called
    SPLL_1PH_SOGI_OSG_COEFF osg_coeff; //!< Orthogonal signal generator coefficient
    SPLL_1PH_SOGI_LPF_COEFF lpf_coeff; //!< Loop filter coeffcient structure
} SPLL_1PH_SOGI;

//! \brief Resets internal storage data of the module
//! \param *spll_obj The SPLL_1PH_SOGI structure pointer
//! \return None
//!
static inline void SPLL_1PH_SOGI_reset(SPLL_1PH_SOGI *spll_obj)
{
    spll_obj->u[0]=(real_t)(0.0);
    spll_obj->u[1]=(real_t)(0.0);
    spll_obj->u[2]=(real_t)(0.0);
    
    spll_obj->osg_u[0]=(real_t)(0.0);
    spll_obj->osg_u[1]=(real_t)(0.0);
    spll_obj->osg_u[2]=(real_t)(0.0);
    
    spll_obj->osg_qu[0]=(real_t)(0.0);
    spll_obj->osg_qu[1]=(real_t)(0.0);
    spll_obj->osg_qu[2]=(real_t)(0.0);
    
    spll_obj->u_Q[0]=(real_t)(0.0);
    spll_obj->u_Q[1]=(real_t)(0.0);
    
    spll_obj->u_D[0]=(real_t)(0.0);
    spll_obj->u_D[1]=(real_t)(0.0);
    
    spll_obj->ylf[0]=(real_t)(0.0);
    spll_obj->ylf[1]=(real_t)(0.0);
    
    spll_obj->fo=(real_t)(0.0);
    
    spll_obj->theta=(real_t)(0.0);
    
    spll_obj->sine=(real_t)(0.0);
    spll_obj->cosine=(real_t)(0.0);
}

//! \brief Calculates the SPLL_1PH_SOGI coefficient
//! \param *spll_obj The SPLL_1PH_SOGI structure
//! \return None
//!
static inline void SPLL_1PH_SOGI_coeff_calc(SPLL_1PH_SOGI *spll_obj)
{
    real_t osgx,osgy,temp, wn;
    wn= spll_obj->fn *(real_t) 2.0f * (real_t) 3.14159265f;
    spll_obj->osg_coeff.osg_k=(real_t)(0.5);
    osgx = (real_t)(wn*spll_obj->delta_t);
    spll_obj->osg_coeff.osg_x=(real_t)(osgx);
    osgy = (real_t)(wn*spll_obj->delta_t*wn*spll_obj->delta_t);
    spll_obj->osg_coeff.osg_y=(real_t)(osgy);
    temp = (real_t)(1.0)/(osgx+osgy+4);
    spll_obj->osg_coeff.osg_b0=((real_t)osgx*temp);
    spll_obj->osg_coeff.osg_b2=((real_t)(-1.0f)*spll_obj->osg_coeff.osg_b0);
    spll_obj->osg_coeff.osg_a1=((real_t)(2*(4-osgy))*temp);
    spll_obj->osg_coeff.osg_a2=((real_t)(osgx-osgy-4)*temp);
    spll_obj->osg_coeff.osg_qb0=((real_t)(real_t(0.5f)*osgy)*temp);
    spll_obj->osg_coeff.osg_qb1=(spll_obj->osg_coeff.osg_qb0*(real_t)(2.0));
    spll_obj->osg_coeff.osg_qb2=spll_obj->osg_coeff.osg_qb0;
}

//! \brief Configures the SPLL_1PH_SOGI module
//! \param *spll_obj The SPLL_1PH_SOGI structure
//! \param acFreq Nominal AC frequency for the SPLL Module
//! \param isrFrequency Frequency at which SPLL module is run
//! \param lpf_b0 B0 coefficient of LPF of SPLL
//! \param lpf_b1 B1 coefficient of LPF of SPLL
//! \return None
//!
static inline void SPLL_1PH_SOGI_config(SPLL_1PH_SOGI *spll_obj,
                         real_t acFreq,
                         real_t isrFrequency,
                         real_t lpf_b0,
                         real_t lpf_b1)
{
    spll_obj->fn=acFreq;
    spll_obj->delta_t=((1)/isrFrequency);

    SPLL_1PH_SOGI_coeff_calc(spll_obj);

    spll_obj->lpf_coeff.b0=lpf_b0;
    spll_obj->lpf_coeff.b1=lpf_b1;
}

//! \brief Run the SPLL_1PH_SOGI module
//! \param *spll_obj The SPLL_1PH_SOGI structure pointer
//! \param acValue AC grid voltage in per unit (pu)
//! \return None
//!
static inline void SPLL_1PH_SOGI_run(SPLL_1PH_SOGI *spll_obj,
                                     real_t acValue)
{
    // Update the spll_obj->u[0] with the grid value
    spll_obj->u[0]=acValue;

    //
    // Orthogonal Signal Generator
    //
    spll_obj->osg_u[0]=(spll_obj->osg_coeff.osg_b0*
                       (spll_obj->u[0]-spll_obj->u[2])) +
                       (spll_obj->osg_coeff.osg_a1*spll_obj->osg_u[1]) +
                       (spll_obj->osg_coeff.osg_a2*spll_obj->osg_u[2]);

    spll_obj->osg_u[2]=spll_obj->osg_u[1];
    spll_obj->osg_u[1]=spll_obj->osg_u[0];

    spll_obj->osg_qu[0]=(spll_obj->osg_coeff.osg_qb0*spll_obj->u[0]) +
                        (spll_obj->osg_coeff.osg_qb1*spll_obj->u[1]) +
                        (spll_obj->osg_coeff.osg_qb2*spll_obj->u[2]) +
                        (spll_obj->osg_coeff.osg_a1*spll_obj->osg_qu[1]) +
                        (spll_obj->osg_coeff.osg_a2*spll_obj->osg_qu[2]);

    spll_obj->osg_qu[2]=spll_obj->osg_qu[1];
    spll_obj->osg_qu[1]=spll_obj->osg_qu[0];

    spll_obj->u[2]=spll_obj->u[1];
    spll_obj->u[1]=spll_obj->u[0];

    //
    // Park Transform from alpha beta to d-q axis
    //
    spll_obj->u_Q[0]=(spll_obj->cosine*spll_obj->osg_u[0]) +
                     (spll_obj->sine*spll_obj->osg_qu[0]);
    spll_obj->u_D[0]=(spll_obj->cosine*spll_obj->osg_qu[0]) -
                     (spll_obj->sine*spll_obj->osg_u[0]);

    //
    // Loop Filter
    //
    spll_obj->ylf[0]=spll_obj->ylf[1] +
                     (spll_obj->lpf_coeff.b0*spll_obj->u_Q[0]) +
                     (spll_obj->lpf_coeff.b1*spll_obj->u_Q[1]);
    spll_obj->ylf[1]=spll_obj->ylf[0];

    spll_obj->u_Q[1]=spll_obj->u_Q[0];

    //
    // VCO
    //
    spll_obj->fo=spll_obj->fn+spll_obj->ylf[0];

    spll_obj->theta=spll_obj->theta + (spll_obj->fo*spll_obj->delta_t)*
                       (real_t)(TAU);

    if(spll_obj->theta>(real_t)(TAU))
    {
        spll_obj->theta=spll_obj->theta - (real_t)(TAU);
        //spll_obj->theta=0;
    }


    spll_obj->sine=(real_t)sin(spll_obj->theta);
    spll_obj->cosine=(real_t)cos(spll_obj->theta);
}
