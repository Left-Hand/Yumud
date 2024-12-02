#pragma once

#include "sys/core/platform.h"
#include <functional>
#include <queue>

#include "sys/math/real.hpp"


namespace ymd{

struct Action {
public:
    using Callback = std::function<void(void)>;
protected:
    Callback func_ = nullptr;
    
    size_t sustain = 0;
    const size_t full;
    volatile bool executed = false;

    virtual void execute(){
        EXECUTE(func_);
    }

    // enum class SpecialActionType{
    //     NONE,
    //     DELAY,
    //     CLEAR,
    //     ABORT
    // };
    
    // virtual SpecialActionType special() const {return SpecialActionType::NONE;}

    bool first() const {
        return executed == false;
    }

    real_t progress() const {
        return real_t(1) - real_t(sustain) / full;
    }

    void kill(){
        sustain = 0;
    }

    real_t time() const {
        return real_t(CLAMP(full - sustain, size_t(0), size_t((1 << GLOBAL_Q )- 5))) / 1000;
    }
public:
    // Action(std::function<void()> &&f, const uint s = 0, const bool _once = true) : func(std::move(f)), sustain(s), full(s), once(_once) {}
    Action(const size_t s, Callback &&f) : func_(std::move(f)), sustain(s), full(s){}

    bool died() const{
        return sustain <= 0;
    }
    // bool is_valid() const {
    //     return sustain > 0;
    // }

    // operator bool() const {
    //     return is_valid();
    // }

    Action& operator--() {
        if (sustain > 0) {
            --sustain;
        }
        return *this;
    }

    size_t remain() const {
        return sustain;
    }

    void invoke(){
        // if(sustain > 0) sustain--;
        // if(once and executed) return;
        if(sustain > 0){
            execute();
            if(sustain) sustain --;
            executed = true;
        }
    }
};

struct DelayAction:public Action{
protected:
    void execute() override {}
public:
    DelayAction( const uint dur):Action(dur, nullptr){}
};



}