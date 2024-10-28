#pragma once



#include "robots/actions/ActionQueue.hpp"

#include "../common/inc.hpp"

namespace gxm{

class MotionModule{
protected:
    ActionQueue actions_;
public:
    MotionModule() = default;

    void addAction(Action * action){actions_ << action;}
};

}