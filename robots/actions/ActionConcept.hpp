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

// protected:
private:
    Callback func_ = nullptr;
    
    int sustain_ = 0;
    const size_t full_;
    volatile bool executed_ = false;
    volatile bool decreased_ = false;
protected:
    virtual void execute(){
        EXECUTE(func_);
    }


    bool first() const {
        return executed_ == false;
    }

    real_t progress() const {
        return real_t(1) - real_t(sustain_) / full_;
    }

    void kill(){
        decreased_ = true;
    }

    real_t time() const {
        return real_t(CLAMP(full_ - sustain_, size_t(0), size_t((1 << GLOBAL_Q )- 5))) / 1000;
    }
public:
    Action(const size_t s, Callback &&f) : func_(std::move(f)), sustain_(MIN(s, INT32_MAX)), full_(sustain_){}

    Action(const Action & other) = delete;
    Action(Action && other) = default;

    bool died() const{
        // return sustain <= 0;
        return decreased_;
    }

    void live(const uint rem){
        sustain_ = rem;
    }


    Action& operator--() {
        if (sustain_ > 0) {
            --sustain_;
        }
        return *this;
    }

    int remain() const {
        return sustain_;
    }

    void invoke(){
        // if(sustain > 0) sustain--;
        // if(once and executed) return;
        if(sustain_ > 0){
            execute();
            // DEBUG_PRINTLN(sustain);
            if(sustain_) sustain_ --;
            if(sustain_ <= 0) decreased_ = true;
            executed_ = true;
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
    DelayAction(const uint dur):Action(dur, nullptr){}
    // DelayAction(const real-t):Action(dur, nullptr){}
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