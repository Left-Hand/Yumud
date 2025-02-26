#pragma once

#include "clock/clock.h"

class TaskTimer {
public:
    // enum MeasureType{
    //     Disabled,
    //     StartTime,
    //     EndTime,
    //     Length,
    //     MaxLength,
    // };

    struct Context{
        uint32_t start_time = 0;
        uint32_t end_time = 0;
        uint32_t length = 0;
        uint32_t max_length = 0;
    };
protected:
    Context context_;
public:
    TaskTimer() = default;

    uint32_t start() {
        return micros();
    }

    void stop(uint32_t start_time) {
        const uint32_t end_time = micros();
        const uint32_t length = end_time - start_time;

        // switch(measure_type){
        //     case MeasureType::Disabled:
        //         break;
        //     case MeasureType::Length:
        //         break;
        //     case MeasureType::StartTime:
        //         break;
        //     case MeasureType::EndTime:
        //         break;
        //     case MeasureType::Length:
        //         break;
        //     case MeasureType::MaxLength:
        //         break;
        //     default:
        //         break;
        // }
        context_.start_time = start_time;
        context_.end_time = end_time;
        context_.length = length;
        context_.max_length = std::max(context_.max_length, length);
    }

    const auto & context() const{return context_;}
};

struct __TaskTimerGuard{
    __TaskTimerGuard(const __TaskTimerGuard&) = delete;
    __TaskTimerGuard(const __TaskTimerGuard&&) = delete;
    void operator=(const __TaskTimerGuard&) = delete;
    void operator=(const __TaskTimerGuard&&) = delete;
    __TaskTimerGuard(TaskTimer& timer) : timer_(timer), start_time(timer.start()) {}
    ~__TaskTimerGuard() { timer_.stop(start_time); }
    
    TaskTimer & timer_;
    uint32_t start_time;
    bool exit_ = false;
};

#define MEASURE_TIME(timer) for (__TaskTimerGuard __task_timer_guard{timer}; !__task_timer_guard.exit_; __task_timer_guard.exit_ = true)