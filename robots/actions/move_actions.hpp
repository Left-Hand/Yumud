#pragma once

#include "actions.hpp"

struct RapidMoveAction:public Action{
protected:
    using Machine = XY_Machine;
    Machine & machine;
    const Vector2 position;

    void execute() override {
        machine.xy_mm(this->position);
    }
public:
    RapidMoveAction(Machine & _machine, const Vector2 & _position, const uint sus):
            Action([this](){this->execute();}, sus), machine(_machine), position(_position){}
};

struct PickAction:public Action{
protected:
    using Machine = Z_Machine;

    Machine & machine;
    const real_t z;

    void execute() override {
        machine.z_mm(this->z);
    }
public:
    PickAction(Machine & _machine, const real_t _z, const uint sus):
            Action([this](){this->execute();}, sus), machine(_machine), z(_z){}
};

struct ClearAction:public Action{
protected:
    using M_Queue = ActionQueue;

    M_Queue & queue;
    void execute() override {}
    SpecialActionType special() const override {
        return SpecialActionType::ABORT;
    }
public:
    ClearAction(M_Queue & _queue):Action(nullptr, 0), queue(_queue){}
};


struct LineInterpolationAction:public Action{
protected:
    using Machine = XY_Machine;

    Machine & machine;
    const Line line;

    void execute() override{
        machine.xy_mm(line.from);
    }
public:
    LineInterpolationAction(Machine & _machine, const Line & _line, const uint sus):
            Action([this](){this->execute();}, sus, false), machine(_machine), line(_line){}
};

