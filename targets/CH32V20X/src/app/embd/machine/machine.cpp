#include "machine.hpp"
#include "../motions/move.hpp"
#include "../motions/pickdrop.hpp"
#include "../motions/reset.hpp"
#include "../../robots/actions/actions.hpp"

void Machine::do_pick(const Vector2 & from){
    actions
    << CombinedAction(
        TrapezoidMoveAction(*this, from)
        , PickAction(*this)
        , HoldAction(*this)
    );
}


void Machine::do_place(const Vector2 & to){
    actions
    << CombinedAction(
        TrapezoidMoveAction(*this, to)
        , PlaceAction(*this)
        , DelayAction(200)
        , ReleaseAction(*this)
        , DelayAction(600)
        , FloatAction(*this)
    ); 
}

void Machine::do_free(){
    // actions += Action([&](){
        x.setTargetCurrent(real_t(0));
        z.setTargetCurrent(real_t(0));
        y.setTargetCurrent(real_t(0));
    // });
}


void Machine::do_idle(const Vector2 & to){

    actions
    << TrapezoidMoveAction(*this, to)
    << FloatAction(*this)
    ;
}

void Machine::toggle_nz(){
    nz(1 - last_nz);
}


void Machine::do_move(const Vector2 & from, const Vector2 & to){
    do_pick(from);
    do_place(to);
}

void Machine::do_home(){
    actions += Action([&](){
        x.setTargetCurrent(real_t(-0.45));
        z.setTargetCurrent(real_t(-0.95));
        y.setTargetCurrent(real_t(-0.45));
    }, 4400);

    actions += Action([&](){
        x.locateRelatively(0);
        y.locateRelatively(0);
        z.locateRelatively(0);
    });

    actions += FloatAction(*this);
    actions += DelayAction(200);
    do_idle();
}




bool Machine::record(){
    if(not trajectory.is_full()){
        trajectory.push(int(x_axis.readMM()), int(y_axis.readMM()), int(z_axis.readMM()), bool(last_nz));
        return true;
    }else{
        return false;
    }
}

void Machine::tick(){
    actions.update();

    using enum MachineState;

    if(millis() % (1000 / trajectory.record_fps) == 0){
        switch(machine_state){
            case TEACH:
                if(not record()) exit_teach();
                break;
            case REPLAY:
                if(not replay()) exit_replay();
                break;
            case NONE:
            default:
                break;
        }
    }
}

bool Machine::replay(){
    const auto & item = trajectory[play_index];
    x_axis.setTargetMM(item.x);
    y_axis.setTargetMM(item.y);
    z_axis.setTargetMM(item.z);
    nz(item.nz);
    play_index++;
    return (play_index < trajectory.size());
}

void Machine::entry_teach(){
    actions << Action([&](){
        x.setTargetTeach(real_t(0.3));
        y.setTargetTeach(real_t(0.3));
        z.setTargetTeach(real_t(0.6));
    });

    trajectory.clear();
    machine_state = MachineState::TEACH;
}

void Machine::exit_teach(){
    do_free();
    machine_state = MachineState::NONE;
}

void Machine::entry_replay(){
    machine_state = MachineState::REPLAY;
    play_index = 0;
}

void Machine::exit_replay(){
    do_free();
    machine_state = MachineState::NONE;
}

void Machine::look(){
    DEBUG_PRINTLN("look");
    if(trajectory.size()) for(uint i = 0; i < trajectory.size(); i++){
        DEBUG_PRINTLN(trajectory[i]);
    }
    DEBUG_PRINTLN(trajectory.size());
    DEBUG_PRINTLN("look down");
}