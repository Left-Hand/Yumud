#include "src/testbench/tb.h"

#include "sys/math/real.hpp"

#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"
#include "sys/clock/clock.h"

#include "cybergear/MotorCyberGear.hpp"

using namespace ymd;
using namespace ymd::rmst;

void rmst_main(){
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    MotorCyberGear motor(can1, 0x01, 0x02);
    while(true){
        // !motor.changeCanId(0);
        
        // !+motor.requestWritePara(0x301, 0x02);
        !motor.ctrl(0, 0, 0, 500, 5);
        delay(10);
    }

    std::terminate();
}