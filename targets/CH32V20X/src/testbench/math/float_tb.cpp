#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/math/float/bf16.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

void float_main(){
    DEBUGGER_INST.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000 
    });

    while(true){
        math::bf16 i = math::bf16(math::sin(clock::time()));
        math::bf16 o = i;

        const auto begin_us = clock::micros();
        for(size_t _ = 0; _ < 10000; _++){
            o = math::bf16(float(math::sin(real_t::from(float(o)))));
        }
        const auto dur_us = clock::micros() - begin_us;
        DEBUG_PRINTLN(real_t(i), real_t(o), dur_us);
    }
}