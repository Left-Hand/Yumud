#pragma once

#include "ActionIntf.hpp"
#include "ActionQueue.hpp"


namespace ymd::robots{

struct CombinedAction final:public ActionBase{
protected:
    using Queue = std::queue<std::unique_ptr<ActionBase>>; 
    Queue action_queue_ = {};
public:
    template<typename... Args,
        std::enable_if_t<(std::is_base_of_v<ActionBase, Args> &&...), int> = 0>
    CombinedAction(Args&&... actions) : 
        ActionBase([this]() { execute(); }, UINT32_MAX, false)
    {
        (action_queue_.emplace(std::make_unique<Args>(
            std::forward<Args>(actions))), ...);
    }

    auto & operator<<(std::unique_ptr<ActionBase> action) {
        action_queue_.emplace(std::move(action));
        return *this;
    }

    auto & operator<<(ActionBase * action) {
        action_queue_.emplace((action));
        return *this;
    }
    
    const auto & queue() const {return action_queue_;}
    size_t pending() const {return action_queue_.size();}

    void CombinedAction::execute() {
        if(action_queue_.empty() == false){
            ActionBase & action = *action_queue_.front();
            action.invoke();
            if (action.died() == true) {
                action_queue_.pop();
            }
        }else{
            live(0);
        }
    }

};



}