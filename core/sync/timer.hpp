#pragma once

#include "core/utils/Option.hpp"

namespace ymd::async{
struct RepeatTimer final{
    explicit constexpr RepeatTimer(Milliseconds duration):
        duration_(duration){;}

    template<typename Fn>
    void invoke_if(Fn && fn){
        const auto now = clock::millis();
        if(last_.count() / duration_.count() != now.count() / duration_.count()){
            std::forward<Fn>(fn)();
            last_invoke_ = now;
        }
        last_ = now;
    }

    [[nodiscard]] Milliseconds since_last_invoke() const {
        const auto now = clock::millis();
        return now - last_invoke_;
    }

    void reset() {
        start_ = Some(clock::millis());
    }
private:
    Milliseconds duration_;
    Milliseconds last_ = 0ms;
    Milliseconds last_invoke_ = 0ms;
    Option<Milliseconds> start_ = None;
};

struct OnceTimer final {
    explicit constexpr OnceTimer(Milliseconds delay) :
        delay_(delay){}

    template<typename Fn>
    void invoke_if(Fn&& fn) {
        if (has_been_expired()) {
            std::forward<Fn>(fn)();
        }
    }

    [[nodiscard]] bool has_been_expired() const {
        if(start_.is_none()) return false;
        return (clock::millis() - start_.unwrap()) >= delay_;
    }

    void reset(Milliseconds new_delay = 0ms) {
        start_ = Some(clock::millis());
        if (new_delay != 0ms) {
            delay_ = new_delay;
        }
    }

private:
    Milliseconds delay_;
    Option<Milliseconds> start_ = None;
};

}