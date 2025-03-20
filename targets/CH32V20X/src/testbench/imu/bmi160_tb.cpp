#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"

#include "drivers/IMU/Axis6/BMI160/bmi160.hpp"

#include "types/quat/Quat.hpp"

using namespace ymd::drivers;

void bmi160_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

    spi1.init(18000000);
    spi1.bind_cs_pin(portA[0], 0);

    using Quat = Quat_t<real_t>;
    
    BMI160 bmi{spi1, 0};
    bmi.init();

    auto & ledr = portC[13];
    auto & ledb = portC[14];
    auto & ledg = portC[15];

    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    portA[7].inana();

    bmi.init();
    while(true){
        // auto pos = ma730.getLapPosition();

        ledr = (millis() % 200) > 100;
        ledb = (millis() % 400) > 200;
        ledg = (millis() % 800) > 400;

        delay(20);
        bmi.update();
        Vector3_t<real_t> acc = bmi.getAcc().unwrap();
        acc.x = acc.x >> 10;
        acc.y = acc.y >> 10;
        acc.z = acc.z >> 10;
        acc.normalize();
        Quat gest = {{0,0,1}, acc};
        DEBUG_PRINTLN(gest.x, gest.y, gest.z, gest.w);
    }
}