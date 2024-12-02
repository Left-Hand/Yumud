#pragma once

#include "sys/core/platform.h"
#include <functional>
#include <queue>

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"


namespace ymd{

struct Action {
public:
    using Vector2 = Vector2_t<real_t>;
    using Vector2i = Vector2_t<int>;

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

    real_t since() const {
        return real_t(CLAMP(full - sustain, size_t(0), size_t((1 << GLOBAL_Q )- 5))) / 1000;
    }
public:
    // Action(std::function<void()> &&f, const uint s = 0, const bool _once = true) : func(std::move(f)), sustain(s), full(s), once(_once) {}
    Action(const size_t s, Callback &&f) : func_(std::move(f)), sustain(s), full(s){}

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

    size_t remain() const {
        return sustain;
    }

    void invoke(){
        // if(sustain > 0) sustain--;
        // if(once and executed) return;
        if(sustain > 0){
            execute();
            sustain --;
            executed = true;
        }
    }
};


}