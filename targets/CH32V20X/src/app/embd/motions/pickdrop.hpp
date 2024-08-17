#pragma once

#include "../../robots/actions/actions.hpp"

struct PickDropAction: public Action{
protected:
    using Machine = Nozzle_Machine;

    Machine & machine;

    virtual void first_do() = 0;

    void execute() override {
        if(first()) first_do();
        if(machine.reached()) abort();
    }
public:
    PickDropAction(Machine & _machine):
            Action([this](){this->execute();}, UINT_MAX, false), machine(_machine){}
};


struct PickAction:public PickDropAction{
protected:

    void first_do() override {
        machine.nz(1);
        machine.z_pick();
    }
public:
    PickAction(Machine & _machine):PickDropAction(_machine){}
};

struct PlaceAction:public PickDropAction{
protected:
    void first_do() override {
        machine.z_place();
    }

public:
    PlaceAction(Machine & _machine):PickDropAction(_machine){}
};

struct HoldAction:public PickDropAction{
protected:

    void first_do() override {
        machine.z_hold();
    }
public:
    HoldAction(Machine & _machine):PickDropAction(_machine){}
};

struct ReleaseAction:public PickDropAction{
protected:

    void first_do() override {
        machine.nz(0);
        machine.z_release();
    }
public:
    ReleaseAction(Machine & _machine):PickDropAction(_machine){}
};

struct FloatAction:public PickDropAction{
protected:

    void first_do() override {

        machine.z_inspect();
    }
public:
    FloatAction(Machine & _machine):PickDropAction(_machine){}
};

struct InspectAction:public PickDropAction{
protected:

    void first_do() override {
        machine.z_idle();
    }
public:
    InspectAction(Machine & _machine):PickDropAction(_machine){}
};
