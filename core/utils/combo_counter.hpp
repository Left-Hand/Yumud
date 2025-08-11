#pragma once

#include <cstdint>

namespace ymd{

struct ComboCounter {
    ComboCounter(const size_t times_threshold) 
        : times_threshold_(times_threshold) {}

    void update(const bool matched) {
        if (matched) {
            current_streak_++;
            if (current_streak_ >= times_threshold_) {
                is_active_ = true;
                just_activated_ = true;
            }
        } else {
            // 只要有一次不匹配，立即重置
            current_streak_ = 0;
            is_active_ = false;
            just_activated_ = false;
        }
    }

    bool is_just_active() {
        if (just_activated_) {
            just_activated_ = false; // 只返回一次 true
            return true;
        }
        return false;
    }

    bool is_active() { return is_active_; }

private:
    size_t times_threshold_;  // 需要连续匹配的次数
    size_t current_streak_ = 0;  // 当前连续匹配次数
    bool is_active_ = false;  // 是否已触发
    bool just_activated_ = false;  // 是否刚刚触发（用于 is_just_active）
};

}