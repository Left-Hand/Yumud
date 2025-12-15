#pragma once

#include "core/utils/Option.hpp"

namespace ymd::async{
struct [[nodiscard]] RepeatTimer final {

    static RepeatTimer from_duration(const Milliseconds duration){
        return RepeatTimer{duration};
    }

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
        next_trigger_ = clock::millis() + duration_;
    }

private:
    Milliseconds duration_;
    Milliseconds last_ = 0ms;
    Milliseconds last_invoke_ = 0ms;
    Milliseconds next_trigger_ = 0ms;

    explicit RepeatTimer(Milliseconds duration):
        duration_(duration),
        next_trigger_(clock::millis() + duration) {}

};

struct [[nodiscard]] OnceTimer final {
    static OnceTimer from_timeout(const Milliseconds timeout){
        return OnceTimer(timeout);
    }

    template<typename Fn>
    void invoke_if(Fn&& fn) {
        if (has_been_expired()) {
            std::forward<Fn>(fn)();
        }
    }

    [[nodiscard]] bool has_been_expired() const {
        if(last_invoke_.is_none()) return false;
        return (clock::millis() - last_invoke_.unwrap()) >= timeout_;
    }

    void reset(Milliseconds timeout = 0ms) {
        last_invoke_ = Some(clock::millis());
        if (timeout != 0ms) {
            timeout_ = timeout;
        }
    }

private:
    Milliseconds timeout_;
    Option<Milliseconds> last_invoke_ = None;

    explicit constexpr OnceTimer(Milliseconds delay) :
        timeout_(delay){}

};

}