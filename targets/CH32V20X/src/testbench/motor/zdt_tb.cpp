#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "robots/vendor/zdt/ZdtStepper.hpp"


void zdt_main(UartHw & logger){
    logger.init(576000, CommStrategy::Blocking);
    DEBUGGER.retarget(&logger);
    DEBUGGER.set_eps(4);
    
    can1.init(1_MHz);

    ZdtMotor motor{can1};
    
    
    delay(10);
    motor.enable();
    delay(10);
    motor.triggerCali();
    while(true){
        motor.enable();
        motor.setTargetPosition(sin(time()));    
        delay(10);
        if(can1.available()) DEBUG_PRINTLN(can1.read());
        // DEBUG_PRINTLN(can1.pending(), can1.getRxErrCnt(), can1.getTxErrCnt());
    }
}