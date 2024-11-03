#include "CombinedAction.hpp"


using namespace yumud;


void CombinedAction::execute() {
    if(action_queue.empty() == false){
        Action & action = *action_queue.front();
        action.invoke();
        if (bool(action) == false) {
            action_queue.pop();
        }
    }else{
        sustain = 0;
    }
}
