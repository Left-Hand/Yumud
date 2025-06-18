#pragma once

struct RepeatTimer final{
    explicit constexpr RepeatTimer(Milliseconds duration):
        duration_(duration){;}

    template<typename Fn>
    void invoke(Fn && fn){
        const auto now = clock::millis();
        if(last_.count() / duration_.count() != now.count() / duration_.count()){
            std::forward<Fn>(fn)();
        }
        last_ = now;
        
    }
private:
    Milliseconds duration_;
    Milliseconds last_ = 0ms;
};

struct OnceTimer final {
    explicit constexpr OnceTimer(Milliseconds delay) :
        delay_(delay){}

    template<typename Fn>
    void invoke(Fn&& fn) {
        if (is_expired()) {
            std::forward<Fn>(fn)();
            has_executed_ = true;
        }
    }

    bool is_expired() const {
        if(start_.is_none()) return false;
        return (clock::millis() - start_.unwrap()) >= delay_;
    }

    void reset(Milliseconds new_delay = 0ms) {
        has_executed_ = false;
        start_ = Some(clock::millis());
        if (new_delay != 0ms) {
            delay_ = new_delay;
        }
    }

private:
    Milliseconds delay_;
    Option<Milliseconds> start_ = None;
    bool has_executed_ = false;
};