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
        bool executed = false;
    };

    virtual void execute(){

        if(once == true && executed == true) return;

        EXECUTE(func);
        executed = true;
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
        return sustain > 0;
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

        if(sustain >= 0){
            execute();
        }
    }
};


