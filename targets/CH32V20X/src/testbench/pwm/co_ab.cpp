#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/utils/default.hpp"
#include "core/math/realmath.hpp"

#include "hal/timer/bipolarity_abstract.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "hal/bus/uart/uarthw.hpp"


using namespace ymd;


static constexpr size_t CHOP_FREQ = 40_KHz;
// #define CHOP_FREQ 200

// #define UART usart1
#define UART hal::usart2

#define TIM_INDEX 1
// #define TIM_INDEX 2
// #define TIM_INDEX 3
// #define TIM_INDEX 4

#define TIM1_USE_CC4 0
// #define TIM1_USE_CC4 1




void co_ab_main(){
    hal::usart2.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });
    DEBUGGER.retarget(&hal::usart2);

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
        .count_freq = hal::NearestFreq(CHOP_FREQ),
        .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
    })        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();

    timer.start();

    pwm_p.init(Default);
    pwm_n.init(Default);

    hal::BipolarityTimerOcConjugate pwm_mirror{pwm_p, pwm_n};

    while(true){
        DEBUG_PRINTLN(clock::millis());
        auto dutycycle = math::sin(6 * clock::time());
        pwm_mirror.set_dutycycle(dutycycle);
    }    
}