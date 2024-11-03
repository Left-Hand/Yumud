#pragma once

#include "sys/core/platform.h"
#include <functional>
#include <queue>

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"


namespace yumud{

struct Action {
public:
    using Vector2 = Vector2_t<real_t>;
    using Vector2i = Vector2_t<int>;
protected:
    std::function<void(void)> func = nullptr;
    struct{
        uint sustain = 0;
        const uint full;
        // bool once = true;
        volatile bool executed = false;
    };

    virtual void execute(){
        EXECUTE(func);
    }

    enum class SpecialActionType{
        NONE,
        DELAY,
        CLEAR,
        ABORT
    };
    
    virtual SpecialActionType special() const {return SpecialActionType::NONE;}

    bool first() const {
        return executed == false;
    }

    real_t ratio() const {
        return real_t(1) - real_t(sustain) / full;
    }

    void abort(){
        sustain = 0;
    }

    real_t since() const {
        return real_t(CLAMP(int(full - sustain),0 , ((1 << GLOBAL_Q )- 5))) / 1000;
    }
public:
    // Action(std::function<void()> &&f, const uint s = 0, const bool _once = true) : func(std::move(f)), sustain(s), full(s), once(_once) {}
    Action(std::function<void()> &&f, const uint s = 1) : func(std::move(f)), sustain(s), full(s){}

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