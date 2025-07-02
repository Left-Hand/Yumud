#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "types/vectors/quat/Quat.hpp"

#include "drivers/IMU/Axis6/BMI160/bmi160.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

#define DBG_UART DEBUGGER_INST

void bmi160_main(){
    DBG_UART.init({576_KHz});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets();
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    spi1.init({18000000});


    using Quat = Quat<real_t>;
    
    BMI160 bmi{{spi1, spi1.attach_next_cs(portA[0]).value()}};
    bmi.init().examine();

    auto & ledr = portC[13];
    auto & ledb = portC[14];
    auto & ledg = portC[15];

    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    portA[7].inana();

    bmi.init().examine();
    while(true){
        ledr = BoolLevel::from((clock::millis() % 200).count() > 100);
        ledb = BoolLevel::from((clock::millis() % 400).count() > 200);
        ledg = BoolLevel::from((clock::millis() % 800).count() > 400);

        clock::delay(20ms);
        bmi.update().examine();
        Vector3<real_t> acc = bmi.read_acc().unwrap();
        acc.x = acc.x >> 10;
        acc.y = acc.y >> 10;
        acc.z = acc.z >> 10;
        acc.normalize();
        Quat gest = Quat::from_shortest_arc({0,0,1}, acc);
        DEBUG_PRINTLN(gest);
    }
}