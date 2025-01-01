#include "spll.hpp"

using namespace ymd;

void Spll::coeff_calc(){
    const real_t delta_t = delta_t_x256 >> 8;
    const real_t wn= this->fn *real_t(TAU);

    const real_t osgx_x256 = (real_t)(wn*delta_t_x256);
    const real_t osgx = (real_t)(wn*delta_t);
    
    const real_t osgy_x256 = (real_t)(wn * delta_t_x256 * wn * delta_t);
    const real_t osgy = (real_t)(wn * delta_t * wn * delta_t);

    const real_t temp = (real_t)(256) / (osgx_x256 + osgy_x256 + 4 * 256);

    this->coeff.osg_b0=((real_t)osgx*temp);
    this->coeff.osg_b2=((real_t)(-1)*this->coeff.osg_b0);
    this->coeff.osg_a1=((real_t)(2 * (4-osgy))*temp);
    this->coeff.osg_a2=((real_t)(osgx-osgy-4)*temp);
    this->coeff.osg_qb0=((real_t)(real_t(0.5f) * osgy) * temp);
    this->coeff.osg_qb1=(this->coeff.osg_qb0 * 2);
    this->coeff.osg_qb2=this->coeff.osg_qb0;
}

void Spll::reset(){
    this->u[0]=0;
    this->u[1]=0;
    this->u[2]=0;
    
    this->osg_u[0]=0;
    this->osg_u[1]=0;
    this->osg_u[2]=0;
    
    this->osg_qu[0]=0;
    this->osg_qu[1]=0;
    this->osg_qu[2]=0;
    
    this->u_Q[0]=0;
    this->u_Q[1]=0;
    
    this->u_D[0]=0;
    this->u_D[1]=0;
    
    this->ylf[0]=0;
    this->ylf[1]=0;
    
    this->fo=0;
    
    this->_theta=0;
    
    this->sine=0;
    this->cosine=0;
}

void Spll::update(const real_t u0){
    // SPLL_1PH_SOGI_run(&spll1,(real_t)u0);
    // Update the this->u[0] with the grid value
    this->u[0] = u0 >> volt_scale_bits;

    //
    // Orthogonal Signal Generator
    //
    this->osg_u[0]=(this->coeff.osg_b0 * (this->u[0]-this->u[2])) +
                    (this->coeff.osg_a1 * this->osg_u[1]) +
                    (this->coeff.osg_a2 * this->osg_u[2]);

    this->osg_u[2]=this->osg_u[1];
    this->osg_u[1]=this->osg_u[0];

    this->osg_qu[0]=(this->coeff.osg_qb0 * this->u[0]) +
                        (this->coeff.osg_qb1 * this->u[1]) +
                        (this->coeff.osg_qb2 * this->u[2]) +
                        (this->coeff.osg_a1 * this->osg_qu[1]) +
                        (this->coeff.osg_a2 * this->osg_qu[2]);

    this->osg_qu[2]=this->osg_qu[1];
    this->osg_qu[1]=this->osg_qu[0];

    this->u[2]=this->u[1];
    this->u[1]=this->u[0];

    //
    // Park Transform from alpha beta to d-q axis
    //
    this->u_Q[0]=(this->cosine * this->osg_u[0]) +
                    (this->sine * this->osg_qu[0]);
    this->u_D[0]=(this->cosine * this->osg_qu[0]) -
                    (this->sine * this->osg_u[0]);

    //
    // Loop Filter
    //
    this->ylf[0]=this->ylf[1] +
                    (this->coeff.b0 * this->u_Q[0]) +
                    (this->coeff.b1 * this->u_Q[1]);
    this->ylf[1]=this->ylf[0];

    this->u_Q[1]=this->u_Q[0];

    //
    // VCO
    //
    this->fo=this->fn+this->ylf[0];

    this->_theta=this->_theta + ((this->fo*this->delta_t_x256)*(real_t)(TAU) >> 8);

    if(this->_theta >= (real_t)(TAU)){
        this->_theta -= (real_t)(TAU);
    }


    this->sine=(real_t)sin(this->_theta);
    this->cosine=(real_t)cos(this->_theta);

    // return ;
}