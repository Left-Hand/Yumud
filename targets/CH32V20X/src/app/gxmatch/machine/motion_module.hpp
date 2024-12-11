#pragma once

#include "robots/actions/ActionQueue.hpp"
#include "../autodrive/Navigator.hpp"

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

    // template<typename T>
    // requires std::is_base_of_v<Action, T>
    // auto & operator<<(T && action){
    //     actions_ << std::make_unique<T>(std::move(action));
    //     return *this;
    // }

    void tick(){
        // DEBUG_PRINTLN("tick");
        actions_.update();
    }

    size_t pending() const {
        return actions_.pending();
    }

    bool done() const{
        return pending() == 0;
    }

    const char * which(){
        return actions_.front()->name();
    }

    void wait(const uint ms){
        *this << new DelayAction(ms);
    }
};

}