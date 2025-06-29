#pragma once

#include "ActionConcept.hpp"
#include <memory>

namespace ymd::robots{


class ActionQueue final{
protected:
    using Element = ActionBase;
    using Warpper = std::unique_ptr<Element>;
    using Queue = std::queue<Warpper>; 

    Queue action_queue_;

    bool clear_req_ = false;
public:
    ActionQueue() = default;


    auto & operator<<(Warpper action) {
        action_queue_.emplace(std::move(action));
        return *this;
    }

    auto & operator<<(Element * action) {
        // action_queue_.emplace(Warpper(action));
        action_queue_.emplace((action));
        return *this;
    }
    
    size_t pending() const {
        return action_queue_.size();
    }

    const auto & front(){
        return action_queue_.front();
    }

    void abort(){
        if(action_queue_.empty() != false){
            action_queue_.pop();
        }
    }

    void clear(){
        clear_req_ = true;
    }

    void update() {
        if(clear_req_){
            action_queue_ = Queue();
            clear_req_ = false;
        }

        if(action_queue_.size()){
            auto & action = *action_queue_.front();
            action.invoke();
            if (action.died() == true) {
                action_queue_.pop();
                // DEBUG_PRINTLN("action poped");
            }else{
                // DEBUG_PRINTLN("action remained", pending(), action.name(), action.remain(), (uint32_t)std::addressof(action));
            }
        }
    }
};

}