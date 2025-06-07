#include "src/testbench/tb.h"
#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"



using namespace ymd;
using namespace ymd::hal;

// 外设：
// UART:576000波特率输出，用于观察信号 
// TIM:CH1和CH2构成A相驱动芯片的两个输入端

static constexpr size_t CHOP_FREQ = 4_KHz;
static constexpr size_t MODU_FREQ = 10;
// #define CHOP_FREQ 200

// #define UART uart1
#define UART hal::uart2

#define TIM_INDEX 1
// #define TIM_INDEX 2
// #define TIM_INDEX 3
// #define TIM_INDEX 4

#define TIM1_USE_CC4 0
// #define TIM1_USE_CC4 1

    
void pfm_zvs_main(){


    UART.init(576000);
    DEBUGGER.retarget(&UART);

    #if TIM_INDEX == 1
    auto & timer = hal::timer1;
    #elif TIM_INDEX == 2
    auto & timer = hal::timer2;
    #elif TIM_INDEX == 3
    auto & timer = hal::timer3;
    #elif TIM_INDEX == 4
    auto & timer = hal::timer4;
    #endif


    auto & pwm_p = timer.oc<1>();
    auto & pwm_n = timer.ocn<1>();

    // timer.init(CHOP_FREQ);
    // timer.init(2_KHz);
    timer.init({20_KHz});
    
    pwm_p.init({
        // .valid_level = LOW
    });
    pwm_n.init({});



    while(true){
        const auto t = clock::time();
        const auto st = sinpu(t * 100);
        pwm_p.set_duty(st * 0.4_r + 0.5_r);

        static constexpr auto arr_base = 144_MHz / 20_KHz;
        const auto arr = uint16_t(arr_base * (1 + 0.5_r * st));
        timer.set_arr(arr);

        const auto dead_zone_ns = uint16_t(1000 * (1 + 0.5_r * st));
        timer.set_deadzone_ns(Nanoseconds(dead_zone_ns));
        DEBUG_PRINTLN(arr);

        clock::delay(300us);
        
    }    
}