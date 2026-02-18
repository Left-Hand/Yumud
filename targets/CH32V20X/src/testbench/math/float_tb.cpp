#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/math/float/bf16.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/hw_singleton.hpp"

using namespace ymd;

void float_main(){
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz), 
    });

    while(true){
        math::bf16 i = math::bf16(math::sin(clock::seconds()));
        math::bf16 o = i;

        const auto begin_us = clock::micros();
        for(size_t _ = 0; _ < 10000; _++){
            o = math::bf16(float(math::sin(iq16::from(float(o)))));
        }
        const auto dur_us = clock::micros() - begin_us;
        DEBUG_PRINTLN(iq16(i), iq16(o), dur_us);
    }
}