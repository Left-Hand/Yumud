#pragma once

#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"

namespace ymd{
struct Evaluator{
    Evaluator() = default;

    template<typename FnIn, typename FnProc>
    static void evaluate_func(const uint times, FnIn && fn_in, FnProc && fn_proc){
        
        const auto begin_m = clock::micros();

        #pragma GCC unroll 20
        for(size_t i = 0; i < times; ++i){
            const auto x = real_t(i) / times;
            std::forward<FnProc>(fn_proc)(x);
        }

        const auto end_m = clock::micros();
        
        DEBUG_PRINTS(real_t((end_m - begin_m).count()) / times, "us per call");
        clock::delay(20ms);
        std::terminate();
    }

    template<typename FnIn, typename FnProc>
    void run_func(const uint32_t f_isr, FnIn && fn_in, FnProc && fn_proc){
        hal::timer1.init({
            // .freq = fs_.expect("you have not set fs yet")
            .freq = f_isr
        });
        hal::timer1.attach(hal::TimerIT::Update, {0,0}, [&](){
            x_ = std::forward<FnIn>(fn_in)(time_);
            y_ = std::forward<FnProc>(fn_proc)(x_);
            time_ += delta_;
        });
    }

    constexpr auto get_xy() const {
        return std::make_tuple(x_,y_);
    }

    void set_fs(const uint32_t fs){
        fs_ = Some(fs);
        time_ = 0;
        delta_ = 1_q24 / fs;
    }

    auto time() const {
        return time_;
    }

    auto delta() const {
        return delta_;
    }

private:
    Option<uint32_t> fs_ = None;
    q24 time_ = 0;
    q24 delta_ = 0;

    q16 x_;
    q16 y_;

    // Microseconds delta_;
};

}