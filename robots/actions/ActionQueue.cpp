#include "ActionQueue.hpp"

using namespace ymd;


void ActionQueue::update() {
    if(clear_req){
        action_queue = Queue();
        clear_req = false;
    }

    if(action_queue.size()){
        auto & action = *action_queue.front();
        action.invoke();
        if (action.died() == true) {
            action_queue.pop();
            // DEBUG_PRINTLN("action poped");
        }else{
            // DEBUG_PRINTLN("action remained", pending(), action.name(), action.remain(), (uint32_t)std::addressof(action));
        }
    }
}