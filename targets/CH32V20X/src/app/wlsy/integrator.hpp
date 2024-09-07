#ifndef __POWER_INTERGRATOR_HPP__

#define __POWER_INTERGRATOR_HPP__

#include "wlsy_inc.hpp"
#include "power.hpp"

namespace WLSY{
using Sys::t;
class Intergrator{
protected:
    real_t intergal;
    real_t start_t;
    real_t last_t;

    bool is_idle = true;
public:
    Intergrator() = default;

    void start(){
        intergal = 0;
        start_t = t;
        last_t = start_t;
        is_idle = false;
    }

    void stop(){
        last_t = t;
        is_idle = true;
    }

    void update(const real_t & value){
        if(is_idle) return;
        real_t t_now = t;
        intergal += (t_now - last_t) * value;
        last_t = t_now;
    }

    real_t getAverage(){
        if(last_t == 0) return 0;
        real_t t_now = last_t;
        real_t dt = t_now - start_t;
        real_t average = intergal / dt;
        return average;
    }

};

}
#endif