#pragma once


#include "robots/actions/ActionConcept.hpp"


namespace gxm{

using Action = ymd::Action;

template<typename T>
struct PickDropAction: public Action{
protected:
    using Machine = T;

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


struct ScaraMoveAction:public Action {


private:
    // void execute() override
};

}