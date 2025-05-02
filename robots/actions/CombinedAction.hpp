#pragma once

#include "ActionConcept.hpp"
#include "ActionQueue.hpp"


namespace ymd::robots{

struct CombinedAction:public Action{
protected:
    using Queue = std::queue<std::unique_ptr<Action>>; 
    Queue action_queue = {};
    void execute() override;
public:
    template<typename... Args,
        std::enable_if_t<(std::is_base_of_v<Action, Args> &&...), int> = 0>
    CombinedAction(Args&&... actions) : Action([this]() { execute(); }, UINT32_MAX, false) {
        (action_queue.emplace(std::make_unique<Args>(std::forward<Args>(actions))), ...);
    }

    auto & operator<<(std::unique_ptr<Action> action) {
        action_queue.emplace(std::move(action));
        return *this;
    }

    auto & operator<<(Action * action) {
        action_queue.emplace((action));
        return *this;
    }
    
    const auto & queue() const {return action_queue;}
    size_t pending() const {return action_queue.size();}
};



}