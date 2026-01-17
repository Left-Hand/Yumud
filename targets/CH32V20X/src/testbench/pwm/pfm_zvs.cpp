#include "src/testbench/tb.h"
#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/default.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/analog/adc/hw_singleton.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"



using namespace ymd;


// 外设：
// UART:576000波特率输出，用于观察信号 
// TIM:CH1和CH2构成A相驱动芯片的两个输入端

static constexpr size_t CHOP_FREQ = 4_KHz;
static constexpr size_t MODU_FREQ = 10;
// #define CHOP_FREQ 200

// #define UART usart1
#define UART hal::usart2

#define TIM_INDEX 1
// #define TIM_INDEX 2
// #define TIM_INDEX 3
// #define TIM_INDEX 4

#define TIM1_USE_CC4 0
// #define TIM1_USE_CC4 1

    
void pfm_zvs_main(){


    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
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

    timer.init({
        .remap = hal::TimerRemap::_0,
        .count_freq = hal::NearestFreq(20_KHz),
        .count_mode = hal::TimerCountMode::Up
    })        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();
    
    pwm_p.init(Default);

    pwm_n.init(Default);

    timer.start();

    while(true){
        const auto t = clock::seconds();
        const auto st = math::sinpu(t * 100);
        pwm_p.set_dutycycle(st * 0.4_r + 0.5_r);

        static constexpr auto arr_base = 144_MHz / 20_KHz;
        const auto arr = uint16_t(arr_base * (1 + 0.5_r * st));
        timer.set_arr(arr);

        const auto dead_zone_ns = Nanoseconds(uint16_t(1000 * (1 + 0.5_r * st)));
        timer.bdtr().set_deadzone(dead_zone_ns);
        DEBUG_PRINTLN(arr);

        clock::delay(300us);
        
    }    
}