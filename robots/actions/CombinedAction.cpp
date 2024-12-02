#include "CombinedAction.hpp"


using namespace ymd;


void CombinedAction::execute() {
    if(action_queue.empty() == false){
        Action & action = *action_queue.front();
        action.invoke();
        if (action.died() == true) {
            action_queue.pop();
        }
    }else{
        sustain = 0;
    }
}
