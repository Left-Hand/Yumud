#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"
#include "core/math/realmath.hpp"
#include "core/stream/ostream.hpp"
#include "core/utils/default.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "core/sdk.hpp"


#define PWM_TB_GPIO
// #define PWM_TB_TIMEROC
// #define PWM_TB_CO


using namespace ymd;


void TIM_InitBkin(const TIM_TypeDef * TIMx){

}
void pwm_main(){
    auto & DBG_UART = hal::usart2;
    DBG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        // .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        // .baudrate = hal::NearestFreq(6000000),
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking,
    });



    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);


}