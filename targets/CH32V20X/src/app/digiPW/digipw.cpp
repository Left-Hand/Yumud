#include "digipw.hpp"



#include "types/real.hpp"
#include "dsp/filter/LowpassFilter.hpp"

#include "hal/gpio/port_virtual.hpp"
#include "hal/exti/exti.hpp"
#include "hal/timer/timer.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "hal/timer/capture/capture_channel.hpp"
#include "hal/timer/timer_oc.hpp"

#include "hal/adc/adcs/adc1.hpp"

#include "drivers/Adc/INA226/ina226.hpp"
#include "drivers/Display/Monochrome/SSD1306/ssd1306.hpp"
#include "drivers/Adc/HX711/HX711.hpp"
#include "drivers/Wireless/Radio/HC12/HC12.hpp"

#include "dsp/controller/PID.hpp"
#include "types/image/painter.hpp"



void digipw_main(){
    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    timer1.init(120'000);
    timer1.initBdtr(100);

    auto & ch = timer1.oc(1);
    auto & chn = timer1.ocn(1);
    auto & en_gpio = portB[0];
}
