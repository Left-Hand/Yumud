#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/string/utils/streamed_string_splitter.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"

#include "src/testbench/tb.h"
#include "core/math/realmath.hpp"


using namespace ymd;
// using namespace ymd::hal;


#define UART hal::uart2
static constexpr uint32_t TIM_FREQ = 5000;
static constexpr uint32_t ISR_FREQ = TIM_FREQ / 2;


void laser_ctl_main(){
    UART.init({576000});
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync();
    DEBUG_PRINTLN("powerup");

    auto & led = hal::portB[8];
    led.outpp(HIGH);




    auto & can = hal::can1;
    can.init({hal::CanBaudrate::_1M});


    [[maybe_unused]] auto & mode1_gpio   = hal::portB[1];
    [[maybe_unused]] auto & phase_gpio   = hal::portA[7];
    phase_gpio.outpp();



    hal::timer3.init({TIM_FREQ, hal::TimerCountMode::CenterAlignedUpTrig});

    auto & pwm = hal::timer3.oc<1>();

    pwm.init({});

    bool duty_is_forward = false;

    auto set_duty = [&](real_t duty){
        duty = CLAMP2(duty, 0.99_r);
        duty_is_forward = duty > 0.0_r;
        phase_gpio = BoolLevel::from(duty_is_forward);
        pwm.set_duty(ABS(duty));
    };



    while(true){
        DEBUG_PRINTLN("laser_ctl_main", clock::time());
        clock::delay(5ms);
        // const auto ctime = clock::time();
        // const auto duty = sinpu(ctime) * 0.5_r;
        // set_duty(duty);
        // clock::delay(1ms);
    }
}
