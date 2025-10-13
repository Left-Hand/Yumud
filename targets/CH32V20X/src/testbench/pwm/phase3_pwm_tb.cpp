#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/Option.hpp"

#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "digipw/pwmgen/interleaved_pwmgen3.hpp"

#include "digipw/SVPWM/svpwm3.hpp"



using namespace ymd;


// https://www.cnblogs.com/wchmcu/p/18781096
// https://www.cnblogs.com/wchmcu/p/18325273

#define UART hal::uart2




void tb1_pwm_always_high(hal::AdvancedTimer & timer){
    // timer.init(2_KHz, TimerCountMode::CenterAlignedDualTrig, false);

    digipw::InterleavedPwmGen3 pwm_gen{timer};
    pwm_gen.init({
        .freq = 20_KHz,
        // .freq = 10_KHz,
        // .freq = 5_KHz,
        .deadzone_ns = 200ns
    });


    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);

    timer.register_nvic<hal::TimerIT::CC4>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::CC4>(EN);
    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            pwm_gen.on_update_isr();

            const auto t = clock::time();

            const auto [st, ct] = sincospu(700 * t);

            static constexpr const real_t depth = 0.7_r;
            const auto uvw_dutycycle = digipw::SVM({st * depth, ct * depth});
            pwm_gen.set_dutycycle(uvw_dutycycle);
            break;
        }
        case hal::TimerEvent::CC4:{
            pwm_gen.on_ch4_isr();
            break;
        }
        default: break;
        }
    });


    while(true){

        // DEBUG_PRINTLN(
        //     real_t(timer.oc(1)),
        //     real_t(timer.oc(2)),
        //     real_t(timer.oc(3))
        // );


        DEBUG_PRINTLN_IDLE(
            // u, 
            // v, 
            // w,
            timer.oc<1>().get_dutycycle()
        );
        // clock::delay(1ms);
        clock::delay(100us);
        // pwm_gen.set_dutycycle({0.2_r, 0.4_r, 0.6_r});
        // pwm_gen.set_dutycycle({0.6_r, 0.8_r, 0.9_r});

        // DEBUG_PRINTLN(t, real_t(pwm_trig_));
        // if(trig_occasion_opt_.is_some())
        //     DEBUG_PRINTLN(trig_occasion_opt_.unwrap().kind());
        // clock::delay(1ms);
    }
}

void phase3pwm_main(void){
    UART.init({576000});
    DEBUGGER.retarget(&UART);
    // tb1();
    // TIM1_Phase_shift_Init2();
    tb1_pwm_always_high(hal::timer1);
}
