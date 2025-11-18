#pragma once

#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"

namespace ymd{
struct TransferSysEvaluator{
    TransferSysEvaluator() = default;

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
        auto & timer = hal::timer1;
        timer.init({
            .count_freq = hal::NearestFreq(f_isr),
            .count_mode = hal::TimerCountMode::Up
        }, EN);

        timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
        timer.enable_interrupt<hal::TimerIT::Update>(EN);
        timer.set_event_handler([&](hal::TimerEvent ev){
            switch(ev){
            case hal::TimerEvent::Update:{
                input_ = std::forward<FnIn>(fn_in)(time_);
                output_ = std::forward<FnProc>(fn_proc)(input_);
                time_ += delta_;
                break;
            }
            default: break;
            }
        });
    }

    constexpr auto get_input_and_output() const {
        return std::make_tuple(input_,output_);
    }

    void set_sample_freq(const uint32_t fs){
        fs_ = Some(fs);
        time_ = 0;
        delta_ = 1_iq24 / fs;
    }

    auto time() const {
        return time_;
    }

    auto delta() const {
        return delta_;
    }

private:
    Option<uint32_t> fs_ = None;
    iq24 time_ = 0;
    iq24 delta_ = 0;

    iq16 input_;
    iq16 output_;

    // Microseconds delta_;
};

}