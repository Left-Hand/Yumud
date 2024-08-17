#pragma once

#include "../../robots/foc/remote/remote.hpp"
#include "../../robots/machine/machine_concepts.hpp"
#include "../../robots/actions/action_queue.hpp"

#include "../teach/teach.hpp"


struct Machine:public XY_Machine, public Nozzle_Machine{
protected:
    static constexpr uint pick_z = 47;
    static constexpr uint hold_z = 25;
    static constexpr uint place_z = 47;
    static constexpr uint release_z = place_z - 15;
    static constexpr uint idle_z = 25;
    static constexpr uint inspect_z = 15;

    void z_pick(){
        z_mm(pick_z + z_base);
    }

    void z_hold(){
        z_mm(hold_z + z_base);
    }

    void z_place(){
        z_mm(place_z + z_base);
    }

    void z_idle(){
        z_mm(idle_z + z_base);
    }

    void z_release(){
        z_mm(release_z + z_base);
    }

    void z_inspect(){
        z_mm(inspect_z + z_base);
    }

    real_t last_nz = 0;

    enum class MachineState{
        NONE,
        TEACH,
        REPLAY
    };

    volatile MachineState machine_state = MachineState::NONE;
    uint play_index = 0;
    bool record();
    bool replay();
public:
    ActionQueue & actions;
    Trajectory & trajectory;
    RemoteFOCMotor & w;
    RemoteFOCMotor & x;
    RemoteFOCMotor & y;
    RemoteFOCMotor & z;
    Machine(
        ActionQueue & _actions,
        Trajectory & _trajectory,
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
        
    ,actions(_actions), trajectory(_trajectory),
    w(_w), x(_x), y(_y), z(_z){;}

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

    static constexpr int z_base = 0;

    void do_move(const Vector2 & from, const Vector2 & to);
    void do_pick(const Vector2 & from);
    void do_place(const Vector2 & to);
    void soft_mm(const Vector2 & to);
    void do_idle(const Vector2 & to = Vector2(25, 155));
    void do_home();
    void do_free();
    void toggle_nz();
    void do_inspect();
    void nz(const real_t duty) override {
        last_nz = duty;
        w.setNozzle(duty);
    }

    void tick();
    void entry_teach();
    void exit_teach();
    void entry_replay();
    void exit_replay();
    void look();
};