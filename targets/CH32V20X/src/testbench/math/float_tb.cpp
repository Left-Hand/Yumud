#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/math/float/bf16.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"

void float_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

    while(true){
        bf16 i = sin(time());
        bf16 o = i;

        auto mic = micros();
        for(size_t _ = 0; _ < 10000; _++){
            o = bf16(float(sin(real_t::from(float(o)))));
        }
        auto dur = micros() - mic;
        DEBUG_PRINTLN(real_t(i), real_t(o), dur);
    }
}