#pragma once

#include "core/utils/Option.hpp"

namespace ymd::async{
struct RepeatTimer final {
    explicit RepeatTimer(Milliseconds duration):
        duration_(duration),
        next_trigger_(clock::millis() + duration) {}

    template<typename Fn>
    void invoke_if(Fn && fn) {
        const auto now = clock::millis();
        if (now >= next_trigger_) {
            std::forward<Fn>(fn)();
            last_invoke_ = now;
            // 计算下一个触发点，考虑可能已经错过多个周期的情况
            next_trigger_ = now + duration_;
        }
        last_ = now;
    }

    [[nodiscard]] Milliseconds since_last_invoke() const {
        return clock::millis() - last_invoke_;
    }

    void reset() {
        start_ = Some(clock::millis());
        next_trigger_ = start_.unwrap_or(clock::millis()) + duration_;
    }

private:
    Milliseconds duration_;
    Milliseconds last_ = 0ms;
    Milliseconds last_invoke_ = 0ms;
    Milliseconds next_trigger_ = 0ms;
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