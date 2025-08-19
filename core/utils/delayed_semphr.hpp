#pragma once

#include "core/clock/clock.hpp"

namespace ymd{

class DelayedSemphr final{
public:
    DelayedSemphr(const Milliseconds delay_ms):
        delay_ms_(delay_ms){;}

    void reset(){
        last_millis_ = std::nullopt;
    }

    void give(){
        last_millis_ = Milliseconds(clock::millis());
    }

    bool take(){
        if(last_millis_.has_value() and last_millis_.value() + delay_ms_ < clock::millis()){
            last_millis_ = std::nullopt;
            return true;
        }
        return false;
    }
private:
    Milliseconds delay_ms_ = 0ms;
    std::optional<Milliseconds> last_millis_ = std::nullopt;
};

}