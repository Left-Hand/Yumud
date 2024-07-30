#pragma once

#include "../../sys/core/platform.h"

#include <queue>

#include "shape.hpp"

struct Action {
protected:

    std::function<void()> func = nullptr;

    struct{
        uint16_t sustain = 0;
        bool once = true;
        bool executed = false;
    }__packed;

    Action& operator--() {
        if (sustain > 0) {
            --sustain;
        }
        return *this;
    }


public:
    Action(std::function<void()> &&f, const uint16_t s = 0, const bool _once = true) : func(std::move(f)), sustain(s), once(_once) {}

    // 前置减法运算符重载

    operator bool() const {
        return sustain > 0;
    }

    void invoke(){

        if(sustain > 0) sustain--;

        if(sustain >= 0){
            if(once == true && executed == true) return;

            EXECUTE(func);
            executed = true;

        }
    }
};

struct ActionQueue {
protected:
    std::queue<Action> action_queue = {};
public:
    ActionQueue() = default;

    // 重载前置+=运算符，接受const引用
    ActionQueue& operator+=(const Action &action) {
        action_queue.push(action);
        return *this;
    }

    // 重载前置+=运算符，接受右值引用，利用移动语义
    ActionQueue& operator+=(Action &&action) {
        action_queue.push(std::move(action));
        return *this;
    }

    ActionQueue& operator<<(const Action &action) {
        action_queue.push(action);
        return *this;
    }

    // 重载前置+=运算符，接受右值引用，利用移动语义
    ActionQueue& operator<<(Action &&action) {
        action_queue.push(std::move(action));
        return *this;
    }
    // 返回队列中等待执行的动作数量
    size_t pending() const {
        return action_queue.size();
    }

    const auto & front(){
        return action_queue.front();
    }

    // 更新队列，执行所有动作直到队列为空或动作的sustain为0
    void update() {
        if(action_queue.size()){
            Action & current_action = action_queue.front();
            current_action.invoke();
            if (bool(current_action) == false) {
                action_queue.pop();
            }
        }
    }
};