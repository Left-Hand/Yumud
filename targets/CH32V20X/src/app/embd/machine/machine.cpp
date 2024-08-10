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
        , DelayAction(500)
        , FloatAction(*this)
    ); 
}


void Machine::do_idle(const Vector2 & to){

    actions
    << TrapezoidMoveAction(*this, to);
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
    do_idle();
}