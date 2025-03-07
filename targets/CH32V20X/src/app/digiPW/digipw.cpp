#include "digipw.hpp"

#include "sys/core/system.hpp"
#include "sys/clock/time.hpp"
#include "sys/debug/debug.hpp"

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


#include "sogi/spll.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;
using namespace ymd::digipw;

#define UART uart2

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

    };

    if(true){
        scexpr size_t times = 10000;

        DEBUG_PRINTLN("--------------");
        DEBUG_PRINTLN("start");

        const auto micros_begin = micros();
        real_t tm = 0;

        for(size_t i = 0; i < times; i++){
            scexpr real_t dt = real_t(1) / isr_freq;
            tm += dt;
    
            raw_theta = real_t(TAU) * frac(ac_freq * tm);
            // raw_theta = real_t(TAU) * frac((ac_freq-4.2_r) * tm);
            u0 = 32.0_r * sin(raw_theta) * (0.05_r * sin(8 * tm) + 1);
            spll.update(u0);
        }

        const auto micros_end = micros();
        DEBUG_PRINT(real_t(micros_end - micros_begin) / times,"us per call");
        DEBUG_PRINTLN(uint32_t(micros_begin), uint32_t(micros_end));
        while(true);
    }

    uint32_t dm = 0;
    timer1.bindCb(TimerIT::Update, [&](){
        auto m = micros();
        run_sogi();
        dm = micros() - m;
    });

    timer1.enableIt(TimerIT::Update, {0,0});

    while(true){
        // DEBUG_PRINTLN_IDLE(raw_theta, spll.theta(), dm);
        DEBUG_PRINTLN(u0, raw_theta, spll.theta());
        delay(1);
    }
}
void digipw_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    /*-----------------------*/

    // test_sogi();
    auto & scl_gpio = portB[15];
    auto & sda_gpio = portB[14];
    
    I2cSw i2csw{scl_gpio, sda_gpio};
    i2csw.init(1000000);
    
    INA226 ina226{i2csw};
    ina226.init(6, 1);
    
        // while(true){
        //     DEBUG_PRINTLN_IDLE(millis());
        // }

    auto & curr_ch = ina226.getCurrChannel();
    auto & volt_ch = ina226.getBusVoltChannel();
    auto & power_ch = ina226.getPowerChannel();

    /*-----------------------*/

    timer1.init(300'000);
    timer1.initBdtr(40);

    auto & ch = timer1.oc(1);
    auto & chn = timer1.ocn(1);
    auto & en_gpio = portB[0];
    auto & led = portA[7];

    en_gpio.outpp();
    led.outpp();

    MP1907 mp1907{ch, chn, en_gpio};

    mp1907.init();
    // mp1907.enable();
    en_gpio = 0;

    /*-----------------------*/

    // BuckConverter buck{curr_ch, volt_ch, mp1907};
    // buck.init();

    iq_t<24> duty = 0.5_r;

    // int a;
    // DEBUG_PRINTLN(a);

    while(true){
        
        ina226.update();
        
        // const auto t = 6 * time();
        // const auto s = sinpu<31>(t);
        // DEBUG_PRINTLN_IDLE(real_t(curr_ch), real_t(volt_ch));
        // DEBUG_PRINTLN_IDLE(real_t(curr_ch), real_t(volt_ch), sin(t), sqrt(t), atan2(cos(t), sin(t)));

        const auto curr_meas = iq_t<24>(real_t(curr_ch));
        const auto curr_targ = 0.04_q24;
        const auto curr_err = curr_targ - curr_meas;
        const auto delta = (curr_err * 0.007_r);
        duty = duty + delta;
        DEBUG_PRINTLN_IDLE(real_t(curr_ch), real_t(volt_ch), real_t(power_ch), duty, ch.cvr());

        // mp1907 = real_t(0.5) + real_t(0.4) * sin(t);
        mp1907 = real_t(0.1);
        //  + real_t(0.4) * sin(t);
        // mp1907 = duty;
        led.toggle();
    }
}
