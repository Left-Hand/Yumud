#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/math/float/bf16.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

void float_main(){
    DEBUGGER_INST.init({576000, CommStrategy::Blocking});

    while(true){
        math::bf16 i = math::bf16(sin(clock::time()));
        math::bf16 o = i;

        const auto mic = clock::micros();
        for(size_t _ = 0; _ < 10000; _++){
            o = math::bf16(float(sin(real_t::from(float(o)))));
        }
        const auto dur = clock::micros() - mic;
        DEBUG_PRINTLN(real_t(i), real_t(o), dur);
    }
}