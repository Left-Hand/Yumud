#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"
#include "core/math/realmath.hpp"
#include "core/stream/ostream.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/pwm/gpio_pwm.hpp"
#include "hal/timer/hw_singleton.hpp"



#define PWM_TB_GPIO
// #define PWM_TB_TIMEROC
// #define PWM_TB_CO


using namespace ymd;


void pwm_tb(OutputStream & logger){
    auto & timer = hal::timer1;
    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        .count_freq = hal::NearestFreq(36000),
        .count_mode = hal::TimerCountMode::Up
    })        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();
    timer.start();
    #ifdef PWM_TB_GPIO
    auto pin = hal::PA<8>();
    hal::GpioPwm pwm{pin};
    pwm.init(32);


    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            pwm.tick();
        }
        default: break;
        }
    });


    #endif

    #ifdef PWM_TB_TIMEROC
    PwmIntf & pwm = timer1.oc(1);
    

    #ifdef PWM_TB_CO
    auto & co = timer1.ocn(1);
    co.init();
    co.setPolarity(HIGH);
    co.setIdleState(HIGH);

    timer1.initBdtr();
    #endif
    #endif

    while(true){
        const auto t = clock::seconds();
        logger.println(t, pwm.cnt(), pwm.cvr());
        pwm.set_dutycycle(0.5_iq16 * iq16(math::sin(4 * t)) + 0.5_iq16);
        clock::delay(100us);
    }

}