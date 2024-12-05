#pragma once

#include "sys/core/platform.h"
#include "sys/debug/debug_inc.h"

#include <functional>
#include <queue>

#include "sys/math/real.hpp"
#include "sys/stream/StringStream.hpp"


namespace ymd{

struct Action {
public:
    using Callback = std::function<void(void)>;
protected:
    Callback func_ = nullptr;
    
    int sustain = 0;
    const size_t full;
    volatile bool executed = false;
    volatile bool decreased_ = false;

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
        decreased_ = true;
    }

    real_t time() const {
        return real_t(CLAMP(full - sustain, size_t(0), size_t((1 << GLOBAL_Q )- 5))) / 1000;
    }
public:
    // Action(std::function<void()> &&f, const uint s = 0, const bool _once = true) : func(std::move(f)), sustain(s), full(s), once(_once) {}
    Action(const size_t s, Callback &&f) : func_(std::move(f)), sustain(MIN(s, INT32_MAX)), full(sustain){}

    Action(const Action & other) = delete;
    Action(Action && other) = default;

    bool died() const{
        // return sustain <= 0;
        return decreased_;
    }


    Action& operator--() {
        if (sustain > 0) {
            --sustain;
        }
        return *this;
    }

    int remain() const {
        return sustain;
    }

    void invoke(){
        // if(sustain > 0) sustain--;
        // if(once and executed) return;
        if(sustain > 0){
            execute();
            // DEBUG_PRINTLN(sustain);
            if(sustain) sustain --;
            if(sustain <= 0) decreased_ = true;
            executed = true;
        }
    }

    virtual const char * name() = 0;
};

#define ACTION_NAME(nm)\
const char * name() override {return #nm; }

struct DelayAction:public Action{
protected:
    void execute() override {}
public:
    DelayAction( const uint dur):Action(dur, nullptr){}
    ACTION_NAME(delay)
};

struct DebugAction:public Action{
protected:
    // String str_;
    void execute() override {
        DEBUG_PRINTLN(String(ss_));
    }

    StringStream ss_;
public:
    DebugAction(const char * str):
        Action(1, nullptr){
            ss_ << str;
        }

    DebugAction(const String & str):
        Action(1, nullptr){
            ss_ << str;
        }

    template <typename... Args>
    DebugAction(Args&&... args):
        Action(1, nullptr){
        (this->ss_ << ... << args);
    }

    ACTION_NAME(debug)
};




}