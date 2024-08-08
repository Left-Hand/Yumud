#pragma once

#include "../../robots/stepper/remote/remote.hpp"
#include "machine_concepts.hpp"

struct Joint{
};

struct Axis{

};

struct Machine:public Cantilever, public Nozzle_Machine{
protected:

    static constexpr real_t x_scale = real_t(1.0)/40;
    static constexpr real_t y_scale = real_t(1.0)/40;
    static constexpr real_t z_scale = real_t(1.0)/2;

    static constexpr uint pick_z = 49;
    static constexpr uint hold_z = 25;
    static constexpr uint place_z = 49;
    static constexpr uint idle_z = 25;

    void x_mm(const real_t _x) override {
        x.setTargetTrapezoid(_x*x_scale);
    }

    void y_mm(const real_t _y) override {
        y.setTargetTrapezoid(_y*y_scale);
    }

public:
    RemoteFOCMotor & w;
    RemoteFOCMotor & x;
    RemoteFOCMotor & y;
    RemoteFOCMotor & z;
    Machine(
        RemoteFOCMotor & _w,
        RemoteFOCMotor & _x,
        RemoteFOCMotor & _y,
        RemoteFOCMotor & _z
    ):w(_w), x(_x), y(_y), z(_z){;}

    RemoteFOCMotor & operator [](const uint8_t index){
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
    void z_mm(const real_t _z) override {
        z.setTargetPosition(_z * z_scale);
    }


    real_t last_z_mm;
    void zt_mm(const real_t _z){
        z.setTargetTrapezoid(_z * z_scale);
        last_z_mm = _z;
        
    }
    void z_pick(){
        zt_mm(pick_z);
    }
    void z_hold(){
        zt_mm(hold_z);
    }
    void z_place(){
        zt_mm(place_z);
    }
    void z_idle(){
        zt_mm(idle_z);
    }


    void nz(const bool en){
        w.setNozzle(en ? real_t(1) : real_t(0));
    }
};