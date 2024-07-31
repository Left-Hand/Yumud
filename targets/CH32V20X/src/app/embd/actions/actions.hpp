#pragma once

#include "action_concept.hpp"
#include "action_queue.hpp"
#include "../machine/machine_concepts.hpp"

struct CombinedAction:public Action{
protected:
    using Queue = std::queue<std::unique_ptr<Action>>; 
    Queue action_queue = {};

    void execute() override {
        if(action_queue.empty() == false){
            const auto & action = action_queue.front();
            action->invoke();
            if (bool(*action) == false) {
                action_queue.pop();
            }
        }else{
            sustain = 0;
        }
    }
public:
    // 使用变参模板的构造函数
    template<typename... Args,
             std::enable_if_t<(std::is_base_of_v<Action, Args> &&...), int> = 0>
    CombinedAction(Args&&... actions) : Action([this]() { execute(); }, 60000) {
        // 使用 fold expression 将所有传入的 actions 添加到队列中
        (action_queue.emplace(std::make_unique<Args>(std::forward<Args>(actions))), ...);
    }
};

struct GotoAction:public Action{
protected:
    XYZ_Machine & machine;
    const Vector2 position;

    void execute() override {
        this->machine.xy_mm(this->position);
    }
public:
    GotoAction(XYZ_Machine & _machine, const Vector2 & _position, const uint16_t sus):Action([this](){this->execute();}, sus), machine(_machine), position(_position){}
};

struct DropAction{
    const Vector2 position;

};