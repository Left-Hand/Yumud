#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/math/float/bf16.hpp"
#include "dsp/filter/EKF.hpp"
// #include "kalman/kalman3.hpp"

// #define UART uart2
#define UART uart2
void ekf_main(){

    
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();
    
    delay(200);

    // AdaptiveEKF<real_t, 2,3> ekf = {};
    // Predict<real_t> predict;
    // Measure<real_t> measure;
    
    // while(true){
    //     ekf.update(measure, Matrix_t<real_t,3,1>{3,4,5});
    //     ekf.predict(predict);
    // }

    while(true);
}