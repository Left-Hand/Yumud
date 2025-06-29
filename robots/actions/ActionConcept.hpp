#pragma once

#include "core/platform.hpp"
#include "core/debug/debug.hpp"
#include "core/math/real.hpp"
#include "core/stream/StringStream.hpp"

#include <functional>
#include <queue>

namespace ymd::robots{

struct ActionBase {
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
        return CLAMP(full_ - sustain_, size_t(0), UINT32_MAX) / 1000;
    }
public:
    ActionBase(const size_t s, Callback &&f) : func_(std::move(f)), sustain_(MIN(s, INT32_MAX)), full_(sustain_){}

    ActionBase(const ActionBase & other) = delete;
    ActionBase(ActionBase && other) = default;
    virtual ~ActionBase() = default;

    bool died() const{
        // return sustain <= 0;
        return decreased_;
    }

    void live(const uint rem){
        sustain_ = rem;
    }


    ActionBase& operator--() {
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


template<typename T>
concept is_action = std::is_base_of_v<ActionBase, T>;


#define ACTION_NAME(nm)\
const char * name() {return #nm; }

struct DelayAction:public ActionBase{
protected:
    void execute() override {}
public:
    DelayAction(const uint dur):ActionBase(dur, nullptr){}
    // DelayAction(const real-t):ActionBase(dur, nullptr){}
    ACTION_NAME(delay)
};

struct DebugAction:public ActionBase{
protected:
    // String str_;
    void execute() override {
        DEBUG_PRINTLN(String(ss_));
    }

    StringStream ss_;
public:
    DebugAction(const char * str):
        ActionBase(1, nullptr){
            ss_ << str;
        }

    DebugAction(const String & str):
        ActionBase(1, nullptr){
            ss_ << str;
        }

    template <typename... Args>
    DebugAction(Args&&... args):
        ActionBase(1, nullptr){
        (this->ss_ << ... << args);
    }

    ACTION_NAME(debug)
};




}