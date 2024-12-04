#pragma once

#include "ActionQueue.hpp"

namespace ymd{

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


struct AbortAction:public Action{
protected:
    using M_Queue = ActionQueue;

    M_Queue & queue;

    void execute() override {}
    SpecialActionType special() const override {
        return SpecialActionType::ABORT;
    }
public:
    AbortAction(M_Queue & _queue):Action(nullptr, 0), queue(_queue){}
};


}