#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/math/float/bf16.hpp"
#include "dsp/filter/EKF.hpp"
#include "hal/bus/uart/uarthw.hpp"
// #include "kalman/kalman3.hpp"

// #define UART usart2
#define UART hal::usart2
void ekf_main(){

    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz), 
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