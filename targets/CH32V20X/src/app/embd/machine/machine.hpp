#pragma once

#include "../../robots/foc/remote/remote.hpp"
#include "../../robots/machine/machine_concepts.hpp"
#include "../../robots/actions/action_queue.hpp"


struct Machine:public XY_Machine, public Nozzle_Machine{
protected:
    void z_pick(){
        z_mm(pick_z);
    }
    void z_hold(){
        z_mm(hold_z);
    }
    void z_place(){
        z_mm(place_z);
    }
    void z_idle(){
        z_mm(idle_z);
    }

    void z_release(){
        z_mm(release_z);
    }

public:
    ActionQueue &actions;
    RemoteFOCMotor & w;
    RemoteFOCMotor & x;
    RemoteFOCMotor & y;
    RemoteFOCMotor & z;
    Machine(
        ActionQueue & _actions,
        RemoteFOCMotor & _w,
        RemoteFOCMotor & _x,
        RemoteFOCMotor & _y,
        RemoteFOCMotor & _z
    ):
    XY_Machine(
        Axis(_x, real_t(1.0/40), {0, 300}),
        Axis(_y, real_t(1.0/40), {0, 200})),

    Nozzle_Machine(
        Axis(_z, real_t(1.0/2), {0, 50}))
        
    ,actions(_actions), w(_w), x(_x), y(_y), z(_z){;}

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


    void do_move(const Vector2 & from, const Vector2 & to);
    void do_pick(const Vector2 & from);
    void do_place(const Vector2 & to);
    void do_idle(const Vector2 & to = Vector2(20, 60));
    void do_home();
    void nz(const real_t duty) override {
        w.setNozzle(duty);
    }
};