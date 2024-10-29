#pragma once

#include "ActionConcept.hpp"
#include <memory>

template<typename T>
concept is_action = std::is_base_of_v<Action, T>;

class ActionQueue {
protected:

    using Queue = std::queue<std::unique_ptr<Action>>; 
    Queue action_queue = {};

    bool clear_req = false;
public:
    ActionQueue() = default;


    auto & operator<<(std::unique_ptr<Action> action) {
        action_queue.emplace(std::move(action));
        return *this;
    }

    auto & operator<<(Action * action) {
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