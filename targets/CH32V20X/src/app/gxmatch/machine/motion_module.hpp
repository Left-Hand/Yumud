#pragma once



#include "robots/actions/ActionQueue.hpp"

#include "../common/inc.hpp"

namespace gxm{

class MotionModule{
protected:
    ActionQueue actions_;
public:
    MotionModule() = default;

    // void addAction(Action * action){actions_ << action;}

    auto & operator<<(Action * action) {
        // action_queue.emplace(Warpper(action));
        actions_ << action;
        return *this;
    }
    void tick(){
        actions_.update();
    }
};

}