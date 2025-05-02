#include "CombinedAction.hpp"


using namespace ymd::robots;


void CombinedAction::execute() {
    if(action_queue.empty() == false){
        Action & action = *action_queue.front();
        action.invoke();
        if (action.died() == true) {
            action_queue.pop();
        }
    }else{
        live(0);
    }
}
