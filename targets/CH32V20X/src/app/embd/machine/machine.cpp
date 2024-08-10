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

void Machine::entry_teach(){
    actions << Action([&](){
        x.setTargetTeach(real_t(0.3));
        y.setTargetTeach(real_t(0.3));
        z.setTargetTeach(real_t(0.6));
    });
}