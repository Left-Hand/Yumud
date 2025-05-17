#pragma once

#include "core/clock/clock.hpp"

namespace ymd{
class TaskTimer final{
public:
    // enum MeasureType{
    //     Disabled,
    //     StartTime,
    //     EndTime,
    //     Length,
    //     MaxLength,
    // };

    struct Context{
        Microseconds start_time;
        Microseconds end_time;
        Microseconds length;
        Microseconds max_length;
    };
protected:
    Context context_;
public:
    TaskTimer() = default;

    Microseconds start() {
        return clock::micros();
    }

    void stop(Microseconds start_time) {
        const auto end_time = clock::micros();
        const auto length = end_time - start_time;

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
    __TaskTimerGuard(TaskTimer& timer) : timer_(timer), start_time_(timer.start()) {}
    ~__TaskTimerGuard() { timer_.stop(start_time_); }
    
private:
    TaskTimer & timer_;
    Microseconds start_time_;
    bool exit_ = false;
};

}
#define MEASURE_TIME(timer) for (__TaskTimerGuard __task_timer_guard{timer}; !__task_timer_guard.exit_; __task_timer_guard.exit_ = true)