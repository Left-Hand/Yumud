#include "digipw.hpp"



#include "dsp/filter/LowpassFilter.hpp"

#include "hal/gpio/port_virtual.hpp"
#include "hal/bus/uart/uarthw.hpp"
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

#include "drivers/Actuator/Bridge/MP1907/mp1907.hpp"

#include "dsp/controller/PID.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "buck/buck.hpp"

#include "sys/core/system.hpp"

using Sys::t;

using namespace ymd;
using namespace ymd::drivers;

void digipw_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    /*-----------------------*/

    auto & scl_gpio = portB[15];
    auto & sda_gpio = portB[14];

    I2cSw i2csw{scl_gpio, sda_gpio};
    i2csw.init(100000);

    INA226 ina226{i2csw};
    ina226.init(real_t(0.006), 5);

    auto & curr_ch = ina226.ch(INA226::Index::CURRENT);
    auto & volt_ch = ina226.ch(INA226::Index::BUS_VOLT);

    /*-----------------------*/

    timer1.init(120'000);
    timer1.initBdtr(100);

    auto & ch = timer1.oc(1);
    auto & chn = timer1.ocn(1);
    auto & en_gpio = portB[0];

    MP1907 mp1907{ch, chn, en_gpio};

    mp1907.init();

    /*-----------------------*/

    Buck buck{curr_ch, volt_ch, mp1907};
    buck.init();
    while(true){
        mp1907 = real_t(0.5) + real_t(0.5) * sin(t);

        ina226.update();
        DEBUG_PRINTLN(real_t(curr_ch), real_t(volt_ch));
    }
}