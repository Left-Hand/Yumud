#pragma once

#include "sys/core/platform.h"

template<arithmetic T>
class Intergrator_t{
protected:
    T intergal;
    T start_t;
    T last_t;

    bool is_idle = true;
public:
    Intergrator_t() = default;

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

    void update(const T & value){
        if(is_idle) return;
        T t_now = t;
        intergal += (t_now - last_t) * value;
        last_t = t_now;
    }

    T getAverage(){
        if(last_t == 0) return 0;
        T t_now = last_t;
        T dt = t_now - start_t;
        T average = intergal / dt;
        return average;
    }

};