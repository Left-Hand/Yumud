#pragma once

#include "odometer.hpp"
#include "dsp/observer/LinearObserver.hpp"
#include "dsp/filter/LowpassFilter.hpp"
#include "sys/core/system.hpp"


namespace yumud::drivers{

class Estimmator{
public:
    using PositionObserver =  LinearObersver_t<real_t, real_t>;
    Odometer & instance;
    PositionObserver positionObserver;
    LowpassFilter_t<real_t, real_t>speed_lpf = LowpassFilter_t<real_t, real_t>(real_t(30.0));
    real_t speed;
    uint32_t dur;
    uint32_t cnt = 0;
public:
    Estimmator(Odometer & _instance, const int & _dur = 20):
        instance(_instance),
        dur(_dur){;}

    virtual void init(){
        instance.init();
    }

    void update(const real_t pos){
        instance.update();

        cnt++;
        if(cnt == dur){
            cnt = 0;
            positionObserver.update(instance.getPosition(), pos);
        }
        speed = speed_lpf.update(positionObserver.getDerivative(), pos);
    }

    real_t getPosition(){
        // return instance.getPosition();
        return positionObserver.predict(t);
    }

    real_t getSpeed(){
        return speed;
    }

    real_t getDirection(){
        return sign(getSpeed());
    }

};


}