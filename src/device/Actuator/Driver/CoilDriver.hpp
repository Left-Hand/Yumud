#ifndef __COIL_DRIVER_HPP__
#define __COIL_DRIVER_HPP__

#include "../Actuator.hpp"

class CoilDriver:public Actuator{
protected:
    virtual void shutdown() = 0;
};

class CoilDriverCurrentSettable: public CoilDriver{
protected:
    using CoilDriver::setDuty;


public:
    virtual void setCurrent(const real_t & current) = 0;
};

class CoilDriverCurrentOpenloop: public CoilDriverCurrentSettable{
protected:
    real_t m_c2d_scale = real_t(1); // current to duty scale
public:
    CoilDriverCurrentOpenloop(const real_t & c2d_scale):m_c2d_scale(c2d_scale) {;}

    void setCurrent(const real_t & current){
        setDuty(m_c2d_scale * current);
    }
};

class CoilDriverCurrentCloseloop: public CoilDriverCurrentSettable{

};
#endif