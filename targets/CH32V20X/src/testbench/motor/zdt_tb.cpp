#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "robots/vendor/zdt/ZdtStepper.hpp"


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

void zdt_main(UartHw & logger){
    logger.init({576000, CommStrategy::Blocking});
    DEBUGGER.retarget(&logger);
    DEBUGGER.set_eps(4);
    
    can1.init(CanBaudrate::_1M);

    ZdtMotor motor{&can1};
    
    
    clock::delay(10ms);
    motor.enable();
    clock::delay(10ms);
    motor.trigger_cali();
    while(true){
        motor.enable();
        motor.set_target_position(sin(clock::time()));    
        clock::delay(10ms);
        if(can1.available()) DEBUG_PRINTLN(can1.read());
        // DEBUG_PRINTLN(can1.pending(), can1.getRxErrCnt(), can1.getTxErrCnt());
    }
}