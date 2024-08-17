#pragma once

#include "../../robots/actions/actions.hpp"

struct HomeAction:public Action{
protected:
    using Machine = XY_Machine;
    Machine & machine;
    const Vector2 position;

    void execute() override {
        machine.xy_mm(this->position);
    }
public:
    HomeAction(Machine & _machine, const Vector2 & _position, const uint sus = 1600):
            Action([this](){this->execute();}, sus), machine(_machine), position(_position){}
};



struct IdleAction:public Action{
protected:
    using Machine = XY_Machine;
    Machine & machine;
    const Vector2 position;

    void execute() override {
        machine.xy_mm(this->position);
    }
public:
    IdleAction(Machine & _machine, const Vector2 & _position, const uint sus = 1600):
            Action([this](){this->execute();}, sus), machine(_machine), position(_position){}
};