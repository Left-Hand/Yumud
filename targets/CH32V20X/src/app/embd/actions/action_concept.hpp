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

public:
    Action(std::function<void()> &&f, const uint16_t s = 0, const bool _once = true) : func(std::move(f)), sustain(s), once(_once) {}

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
    virtual void execute(){

        if(once == true && executed == true) return;

        EXECUTE(func);
        executed = true;
    }

    void invoke(){
        if(sustain > 0) sustain--;

        if(sustain >= 0){
            execute();
        }
    }
};


