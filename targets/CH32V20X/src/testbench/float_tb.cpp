#include "tb.h"

#include "sys/math/float/bf16.hpp"
#include "sys/debug/debug_inc.h"

void float_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    while(true){
        bf16 i = sin(t);
        bf16 o = i;

        auto mic = micros();
        for(size_t _ = 0; _ < 10000; _++){
            o = bf16(float(sin(real_t::from(float(o)))));
        }
        auto dur = micros() - mic;
        DEBUG_PRINTLN(real_t(i), real_t(o), dur);
    }
}