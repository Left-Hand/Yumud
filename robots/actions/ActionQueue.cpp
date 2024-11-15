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
        if (bool(action) == false) {
            action_queue.pop();
        }
    }
}