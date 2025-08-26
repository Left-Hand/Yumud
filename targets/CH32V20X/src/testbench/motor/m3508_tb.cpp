#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/timer/instance/timer_hw.hpp"

#include "types/transforms/Basis.hpp"

#include "robots/vendor/DJI/M3508/m3508.hpp"
#include "robots/vendor/DJI/DR16/DR16.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::robots;

void m3508_main(){
    static constexpr size_t CB_FREQ = 200;

    auto & led = portC[14];
    led.outpp(HIGH);

    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

    can1.init({CanBaudrate::_1M, CanMode::Normal});

    M3508Port port{can1};
    
    timer3.init({CB_FREQ});

    timer3.bind_cb(TimerIT::Update, [&](){
        port.tick();
    });
    timer3.enable_it(TimerIT::Update, {0,0});

    auto & motor = port[4];
    auto & motor2 = port[1];

    motor.init();
    motor2.init();

    while(true){
        // port.setTargetCurrent();
        // real_t targ_curr = real_t(1.5) * sin(real_t(TAU/4) * t);
        // real_t target = 3 * sin(t);
        real_t target = 3 * clock::time();
        // real_t targ_curr = 4 * sin( t*real_t(1.7));
        // real_t targ_curr = 0;
        // motor.setTargetCurrent(targ_curr);
        motor.set_target_position(target);   
        motor2.set_target_position(target);   
        // motor.setTargetCurrent(0);   
        // port[2].setTargetCurrent(max_curr * cos(t));   
        // port[3].setTargetCurrent(max_curr * -sin(t));   
        // port[4].setTargetCurrent(max_curr * -cos(t));

        // port.tick();

        // DEBUGGER.println(port[1].getCurrent(), port[2].getCurrent(), port[3].getCurrent(), port[4].getCurrent(), can1.read()); 
        // auto msg = CanMsg{0x1ff, std::tuple<int16_t, int16_t, int16_t, int16_t>(data, data, data, data)};
        // can1.write(msg);
        // can1.write({0x2fe, std::tuple<int16_t, int16_t, int16_t, int16_t>(5000, 5000, 5000, 5000)});
        // clock::delay(3ms);
        led = BoolLevel::from((clock::millis().count() % 400) > 200);

        // static real_t last_pos = 0;
        // DEBUG_PRINTLN(std::setprecision(3), target, motor.getPosition(), motor.getSpeed(), motor.getCurrent(), motor.delta());
        // DEBUG_PRINTLN(std::setprecision(3), real_t(1.2), std::hex, std::showbase, std::setprecision(2) ,0x23, basis);
        // clock::delay(10ms);
        // last_pos = motor.getPosition();
    }
}