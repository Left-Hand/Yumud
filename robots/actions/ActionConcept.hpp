#pragma once

#include "core/platform.hpp"
#include "core/debug/debug.hpp"
#include "core/math/real.hpp"

#include <functional>
#include <queue>

namespace ymd::robots{

struct ActionBase {
public:
    using Callback = std::function<void(void)>;


protected:
    virtual void execute(){
        EXECUTE(func_);
    }


    bool first() const {
        return is_executed_ == false;
    }

    iq16 progress() const {
        return iq16(1) - iq16(sustain_) / full_;
    }

    void kill(){
        is_finished_ = true;
    }

    iq16 time() const {
        return CLAMP(full_ - sustain_, size_t(0), UINT32_MAX) / 1000;
    }
public:
    ActionBase(const size_t s, Callback &&f) : 
        func_(std::move(f)), 
        sustain_(MIN(s, INT32_MAX)), full_(sustain_){}

    ActionBase(const ActionBase & other) = delete;
    ActionBase(ActionBase && other) = default;

    virtual ~ActionBase() = default;

    bool is_finished() const{
        // return sustain <= 0;
        return is_finished_;
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

    int32_t remain() const {
        return sustain_;
    }

    void invoke(){
        // if(sustain > 0) sustain--;
        // if(once and executed) return;
        if(sustain_ > 0){
            execute();
            // DEBUG_PRINTLN(sustain);
            if(sustain_) sustain_ --;
            if(sustain_ <= 0) is_finished_ = true;
            is_executed_ = true;
        }
    }

    virtual const char * name() = 0;
private:
    Callback func_ = nullptr;
    
    int32_t sustain_ = 0;
    const size_t full_;
    volatile bool is_executed_ = false;
    volatile bool is_finished_ = false;
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


}