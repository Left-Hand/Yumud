#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/timer/hw_singleton.hpp"

#include "algebra/transforms/Basis.hpp"

#include "robots/vendor/DJI/M3508/m3508.hpp"
#include "robots/vendor/DJI/DR16/DR16.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

using namespace ymd::robots::dji::m3508;

#if 0
void m3508_main(){
    static constexpr size_t CB_FREQ = 200;

    auto led = hal::PC<14>();
    led.outpp(HIGH);

    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

    hal::can1.init({
        .remap = 0,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M)
    });

    M3508Port port{hal::can1};
    
    auto & timer = hal::timer3;
    timer.init({
        .count_freq = hal::NearestFreq(CB_FREQ),
        .count_mode = hal::TimerCountMode::Up
    }, EN);

    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            port.tick();
            break;
        }
        default: break;
        }
    });

    auto & motor = port[4];
    auto & motor2 = port[1];

    motor.init();
    motor2.init();

    while(true){
        // port.setTargetCurrent();
        // iq16 targ_curr = iq16(1.5) * sin(iq16(TAU/4) * t);
        // iq16 target = 3 * sin(t);
        iq16 target = 3 * clock::seconds();
        // iq16 targ_curr = 4 * sin( t*iq16(1.7));
        // iq16 targ_curr = 0;
        // motor.setTargetCurrent(targ_curr);
        motor.set_target_position(target);   
        motor2.set_target_position(target);   
        // motor.setTargetCurrent(0);   
        // port[2].setTargetCurrent(max_curr * cos(t));   
        // port[3].setTargetCurrent(max_curr * -sin(t));   
        // port[4].setTargetCurrent(max_curr * -cos(t));

        // port.tick();

        // DEBUGGER.println(port[1].getCurrent(), port[2].getCurrent(), port[3].getCurrent(), port[4].getCurrent(), can1.read()); 
        // auto can_frame = BxCanFrame{0x1ff, std::tuple<int16_t, int16_t, int16_t, int16_t>(data, data, data, data)};
        // can1.write(can_frame);
        // can1.write({0x2fe, std::tuple<int16_t, int16_t, int16_t, int16_t>(5000, 5000, 5000, 5000)});
        // clock::delay(3ms);
        led = BoolLevel::from((clock::millis().count() % 400) > 200);

        // static iq16 last_pos = 0;
        // DEBUG_PRINTLN(std::setprecision(3), target, motor.getPosition(), motor.getSpeed(), motor.getCurrent(), motor.delta());
        // DEBUG_PRINTLN(std::setprecision(3), iq16(1.2), std::hex, std::showbase, std::setprecision(2) ,0x23, basis);
        // clock::delay(10ms);
        // last_pos = motor.getPosition();
    }
}

#endif