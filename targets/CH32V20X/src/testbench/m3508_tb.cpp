#include "tb.h"

#include "sys/debug/debug_inc.h"
#include "robots/DJI/M3508/m3508.hpp"
#include "robots/DJI/DR16/DR16.hpp"
#include "hal/timer/instance/timer_hw.hpp"

void m3508_main(){
    scexpr real_t max_curr = real_t(1.8);
    scexpr size_t cb_freq = 50;

    auto & led = portC[14];
    led.outpp(1);

    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);
    can1.init(1_M);

    M3508Port port{can1};
    
    timer3.init(cb_freq);
    timer3.bindCb(TimerUtils::IT::Update, [&](){
        port.tick();
    });

    while(true){
        port[1].setTargetCurrent(max_curr * sin(t));   
        port[2].setTargetCurrent(max_curr * cos(t));   
        port[3].setTargetCurrent(max_curr * -sin(t));   
        port[4].setTargetCurrent(max_curr * -cos(t));

        DEBUGGER.println(port[1].getCurrent(), port[2].getCurrent(), port[3].getCurrent(), port[4].getCurrent()); 
        // auto msg = CanMsg{0x1fe, std::tuple<int16_t, int16_t, int16_t, int16_t>(data, data, data, data)};
        // can1.write(msg);
        // delay(1);
        // can1.write({0x2fe, std::tuple<int16_t, int16_t, int16_t, int16_t>(5000, 5000, 5000, 5000)});
        // delay(10);
        // led = (millis() % 400) > 200;
        // DEBUG_PRINTLN(can1.pending(), can1.available(), msg, can1.read());
    }
}