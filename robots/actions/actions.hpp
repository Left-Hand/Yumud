#pragma once

#include "action_concept.hpp"
#include "action_queue.hpp"
#include "robots/machine/machine_concepts.hpp"

struct CombinedAction:public Action{
protected:
    using Queue = std::queue<std::unique_ptr<Action>>; 
    Queue action_queue = {};
    void execute() override {
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
public:
    template<typename... Args,
             std::enable_if_t<(std::is_base_of_v<Action, Args> &&...), int> = 0>
    CombinedAction(Args&&... actions) : Action([this]() { execute(); }, UINT_MAX, false) {
        (action_queue.emplace(std::make_unique<Args>(std::forward<Args>(actions))), ...);
    }

    template<is_action T>
    CombinedAction& operator+=(const T &action) {
        action_queue.push(std::make_unique<T>(action));
        return *this;
    }

    template<is_action T>
    CombinedAction& operator+=(T &&action) {
        action_queue.push(std::make_unique<T>(std::forward<T>(action)));
        return *this;
    }

    template<is_action T>
    CombinedAction& operator<<(const T &action) {
        action_queue.push(std::make_unique<T>(action));
        return *this;
    }

    template<is_action T>
    CombinedAction& operator<<(T &&action) {
        action_queue.push(std::make_unique<T>(std::forward<T>(action)));
        return *this;
    }

    const auto & queue() const {return action_queue;}
    size_t pending() const {return action_queue.size();}
};

struct ClearAction:public Action{
protected:
    using M_Queue = ActionQueue;

    M_Queue & queue;
    void execute() override {}
    SpecialActionType special() const override {
        return SpecialActionType::ABORT;
    }
public:
    ClearAction(M_Queue & _queue):Action(nullptr, 0), queue(_queue){}
};


struct AbortAction:public Action{
protected:
    using M_Queue = ActionQueue;

    M_Queue & queue;

    void execute() override {}
    SpecialActionType special() const override {
        return SpecialActionType::ABORT;
    }
public:
    AbortAction(M_Queue & _queue):Action(nullptr, 0), queue(_queue){}
};

struct DelayAction:public Action{
protected:

    void execute() override {}
    SpecialActionType special() const override {
        return SpecialActionType::DELAY;
    }
public:
    DelayAction( const uint dur):Action(nullptr, dur){}
};