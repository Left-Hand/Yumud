#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/math/float/bf16.hpp"
#include "dsp/filter/EKF.hpp"
#include "hal/bus/uart/uarthw.hpp"
// #include "kalman/kalman3.hpp"

// #define UART uart2
#define UART hal::uart2
void ekf_main(){

    DEBUGGER_INST.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000 
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    
    clock::delay(200ms);

    // AdaptiveEKF<real_t, 2,3> ekf = {};
    // Predict<real_t> predict;
    // Measure<real_t> measure;
    
    // while(true){
    //     ekf.update(measure, Matrix<real_t,3,1>{3,4,5});
    //     ekf.predict(predict);
    // }

    while(true);
}