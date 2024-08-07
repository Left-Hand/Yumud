#pragma once

#include "../../sys/core/platform.h"

#include <queue>

#include "shape.hpp"

struct Action {
protected:
    std::function<void()> func = nullptr;
    struct{
        uint sustain = 0;
        bool once = true;
        volatile bool executed = false;
    };

    virtual void execute(){
        EXECUTE(func);
    }

    enum class SpecialActionType{
        NONE,
        CLEAR,
        ABORT
    };
    
    virtual SpecialActionType special() const {return SpecialActionType::NONE;}

public:
    Action(std::function<void()> &&f, const uint s = 0, const bool _once = true) : func(std::move(f)), sustain(s), once(_once) {}

    bool is_valid() const {
        return sustain > 0;
    }

    operator bool() const {
        return is_valid();
    }

    Action& operator--() {
        if (sustain > 0) {
            --sustain;
        }
        return *this;
    }

    auto remain() const {
        return sustain;
    }

    void invoke(){
        if(sustain > 0) sustain--;
        if(once and executed) return;
        if(sustain >= 0){
            execute();
            executed = true;
        }
    }
};


