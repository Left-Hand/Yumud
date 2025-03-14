#include "src/testbench/tb.h"
#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"

#include "sys/math/float/bf16.hpp"
#include "dsp/filter/EKF.hpp"

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