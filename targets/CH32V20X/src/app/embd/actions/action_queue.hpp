#pragma once

#include "action_concept.hpp"

template<typename T>
concept is_action = std::is_base_of_v<Action, T>;

struct ActionQueue {
protected:

    using Queue = std::queue<std::unique_ptr<Action>>; 
    Queue action_queue = {};

public:
    ActionQueue() = default;


    // Overload for lvalue references
    template<is_action T>
    ActionQueue& operator+=(const T &action) {
        action_queue.push(std::make_unique<T>(action));
        return *this;
    }

    // Overload for rvalue references
    template<is_action T>
    ActionQueue& operator+=(T &&action) {
        action_queue.push(std::make_unique<T>(std::forward<T>(action)));
        return *this;
    }

    // Overload for lvalue references using '<<' operator
    template<is_action T>
    ActionQueue& operator<<(const T &action) {
        action_queue.push(std::make_unique<T>(action));
        return *this;
    }

    // Overload for rvalue references using '<<' operator
    template<is_action T>
    ActionQueue& operator<<(T &&action) {
        action_queue.push(std::make_unique<T>(std::forward<T>(action)));
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
        action_queue = Queue();
    }

    void update() {
        if(action_queue.size()){
            const auto & action = action_queue.front();
            action->invoke();
            if (bool(*action) == false) {
                action_queue.pop();
            }
        }
    }
};