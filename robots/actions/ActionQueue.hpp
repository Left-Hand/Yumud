#pragma once

#include "ActionConcept.hpp"
#include <memory>

namespace ymd{


template<typename T>
concept is_action = std::is_base_of_v<Action, T>;

class ActionQueue {
protected:
    using Element = Action;
    using Warpper = std::unique_ptr<Element>;
    using Queue = std::queue<Warpper>; 

    Queue action_queue = {};

    bool clear_req = false;
public:
    ActionQueue() = default;


    auto & operator<<(Warpper action) {
        action_queue.emplace(std::move(action));
        return *this;
    }

    auto & operator<<(Element * action) {
        // action_queue.emplace(Warpper(action));
        action_queue.emplace((action));
        return *this;
    }
    
    size_t pending() const {
        return action_queue.size();
    }

    const auto & front(){
        return action_queue.front();
    }

    void abort(){
        if(action_queue.empty() != false){
            action_queue.pop();
        }
    }

    void clear(){
        clear_req = true;
    }

    void update();


};

}