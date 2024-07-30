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

    void xy_mm(const Vector2 & v){
        x_mm(v.x);
        y_mm(v.y);
    }

    void x_mm(const real_t _x){
        x.setTargetTrapezoid(_x*x_scale);
    }

    void y_mm(const real_t _y){
        y.setTargetTrapezoid(_y*y_scale);
    }

    void z_mm(const real_t _z){
        z.setTargetPosition(_z * z_scale);
    }

    void nz(const bool en){
        w.setNozzle(en);
    }
};