#include "digipw.hpp"
#include <ostream>


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
#include "sogi/spll.hpp"

using Sys::t;

using namespace ymd;
using namespace ymd::drivers;

void test_sogi(){
    scexpr int ac_freq = 50;
    // scexpr int ac_freq = 25;
    // scexpr int ac_freq = 5;
    // scexpr int isr_freq = 16384/4;
    // scexpr int isr_freq = 16384;
    scexpr int isr_freq = 8192;

    Spll spll = {
        isr_freq, ac_freq,
        // 33,-32
    };

    real_t raw_theta;
    real_t u0;


    timer1.init(isr_freq);

    auto run_sogi = [&](){
        static real_t tm = 0;
        scexpr real_t dt = real_t(1) / isr_freq;
        tm += dt;

        raw_theta = real_t(TAU) * frac(ac_freq * tm);
        // raw_theta = real_t(TAU) * frac((ac_freq-4.2_r) * tm);
        u0 = 32.0_r * sin(raw_theta) * (0.05_r * sin(8 * tm) + 1);
        spll.update(u0);
    };

    if(false){
        scexpr size_t times = 10000;

        DEBUG_PRINTLN("--------------");
        DEBUG_PRINTLN("start");

        uint32_t t0 = micros();
        for(size_t i = 0; i < times; i++){
            run_sogi();
        }

        DEBUG_PRINTLN(real_t(micros() - t0) / times, "us per call");
        while(true);
    }

    uint32_t dm = 0;
    timer1.bindCb(TimerUtils::IT::Update, [&](){
        auto m = micros();
        run_sogi();
        dm = micros() - m;
    });

    timer1.enableIt(TimerUtils::IT::Update, {0,0});

    while(true){
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(raw_theta, spll.theta(), dm);
        DEBUG_PRINTLN(u0, raw_theta, spll.theta());
        delay(1);
    }
}
void digipw_main(){
    uart2.init(576000);
    DEBUGGER.change(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");
    /*-----------------------*/

    test_sogi();
    auto & scl_gpio = portB[15];
    auto & sda_gpio = portB[14];

    I2cSw i2csw{scl_gpio, sda_gpio};
    i2csw.init(100000);

    INA226 ina226{i2csw};
    ina226.init(real_t(0.006), 5);

    auto & curr_ch = ina226.currChannel();
    auto & volt_ch = ina226.voltChannel();

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
