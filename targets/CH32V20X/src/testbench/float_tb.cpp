#include "tb.h"
#include "types/float/bf16.hpp"

void float_main(){
    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    while(true){
        bf16 i = sin(t);
        bf16 o = i;

        auto mic = micros();
        for(size_t _ = 0; _ < 10000; _++){
            o = bf16(float(sin(real_t(float(o)))));
        }
        auto dur = micros() - mic;
        DEBUG_PRINTLN(real_t(i), real_t(o), dur);
    }
}