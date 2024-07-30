#pragma once

#include "remote/remote.hpp"

struct Machine{
protected:

    static constexpr real_t x_scale = 1.0/40;
    static constexpr real_t y_scale = 1.0/40;
    static constexpr real_t z_scale = 1.0/2;
public:
    RemoteStepper & w;
    RemoteStepper & x;
    RemoteStepper & y;
    RemoteStepper & z;
    Machine(
        RemoteStepper & _w,
        RemoteStepper & _x,
        RemoteStepper & _y,
        RemoteStepper & _z
    ):w(_w), x(_x), y(_y), z(_z){;}

    RemoteStepper & operator [](const uint8_t index){
        switch(index){
            case 0:
                return w;
            case 1:
                return x;
            case 2:
                return y;
            case 3:
                return z;
            default:
                return w;
        }
    }
};