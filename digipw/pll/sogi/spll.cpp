#include "spll.hpp"
#include <tuple>
#include "core/math/realmath.hpp"

using namespace ymd::digipw;

void Spll::coeff_calc(){
    const real_t delta_t = delta_t_x256 >> 8;
    const real_t wn= fn *real_t(TAU);

    const real_t osgx_x256 = (real_t)(wn*delta_t_x256);
    const real_t osgx = (real_t)(wn*delta_t);
    
    const real_t osgy_x256 = (real_t)(wn * delta_t_x256 * wn * delta_t);
    const real_t osgy = (real_t)(wn * delta_t * wn * delta_t);

    const real_t temp = (real_t)(256) / (osgx_x256 + osgy_x256 + 4 * 256);

    coeff.osg_b0=((real_t)osgx*temp);
    coeff.osg_b2=((real_t)(-1)*coeff.osg_b0);
    coeff.osg_a1=((real_t)(2 * (4-osgy))*temp);
    coeff.osg_a2=((real_t)(osgx-osgy-4)*temp);
    coeff.osg_qb0=((real_t)(real_t(0.5f) * osgy) * temp);
    coeff.osg_qb1=(coeff.osg_qb0 * 2);
    coeff.osg_qb2=coeff.osg_qb0;
}

void Spll::reset(){
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

void Spll::update(const real_t u0){
    // SPLL_1PH_SOGI_run(&spll1,(real_t)u0);
    // Update the u[0] with the grid value
    u[0] = u0 >> volt_scale_bits;

    //
    // Orthogonal Signal Generator
    //
    osg_u[0] = 
        (coeff.osg_b0 * (u[0]-u[2])) +
        (coeff.osg_a1 * osg_u[1]) +
        (coeff.osg_a2 * osg_u[2]);

    osg_u[2] = osg_u[1];
    osg_u[1] = osg_u[0];

    osg_qu[0]=
        (coeff.osg_qb0 * u[0]) +
        (coeff.osg_qb1 * u[1]) +
        (coeff.osg_qb2 * u[2]) +
        (coeff.osg_a1 * osg_qu[1]) +
        (coeff.osg_a2 * osg_qu[2]);

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
        (coeff.b0 * u_Q[0]) +
        (coeff.b1 * u_Q[1]);
    ylf[1] = ylf[0];

    u_Q[1] = u_Q[0];

    fo = fn + ylf[0];

    _theta_x256 =_theta_x256 + (fo*(delta_t_x256*(real_t)(TAU)));

    if(_theta_x256 >= (real_t)(TAU * 256)){
        _theta_x256 -= (real_t)(TAU * 256);
    }

    auto [sin_val, cos_val] = sincos(_theta_x256);
    sine = sin_val;
    cosine = cos_val;
}