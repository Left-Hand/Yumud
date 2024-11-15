#pragma once

#include "robots/actions/actions.hpp"
#include "gxmatch/machine/actuator/zaxis.hpp"

namespace gxm::ZaxisUtils{

using Machine = gxm::Zaxis;

struct PickDropAction: public Action{
protected:

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

}