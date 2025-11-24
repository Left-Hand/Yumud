#include "digipw.hpp"

#include "core/system.hpp"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"


#include "hal/gpio/vport.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/exti/exti.hpp"
#include "hal/timer/timer.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "primitive/pwm_channel.hpp"
#include "hal/timer/capture/capture_channel.hpp"
#include "hal/timer/timer_oc.hpp"

#include "hal/analog/adc/adcs/adc1.hpp"

#include "drivers/Adc/INA226/ina226.hpp"
#include "drivers/Display/Monochrome/SSD1306/ssd1306.hpp"
#include "drivers/Adc/HX711/HX711.hpp"
#include "drivers/Wireless/Radio/HC12/HC12.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "digipw/spll/spll_1ph_sogi_iq.hpp"


#include "core/math/realmath.hpp"


using namespace ymd;

using namespace ymd::drivers;
using namespace ymd::digipw;

#define UARt DEBUGGER_INST


void test_sogi(){
    static constexpr size_t ac_freq = 50;
    // static constexpr size_t ac_freq = 25;
    // static constexpr size_t ac_freq = 5;
    // static constexpr size_t isr_freq = 16384/4;
    // static constexpr size_t isr_freq = 16384;
    static constexpr size_t isr_freq = 8192;
    auto & timer = hal::timer1;
    Spll spll = {
        typename Spll::Config{
            .fs = isr_freq,
            .ac_freq = ac_freq, 
            .b0_lpf = 222.2862_r,
            .b1_lpf = -222.034_r,
        }
    };

    Angle<uq32> raw_angle;
    iq16 u0;


    hal::timer1.init({
        .count_freq = hal::NearestFreq(isr_freq),
        .count_mode = hal::TimerCountMode::Up
    }, EN);

    auto run_sogi = [&](){

    };

    if(true){
        static constexpr size_t times = 10000;

        DEBUG_PRINTLN("--------------");
        DEBUG_PRINTLN("start");

        const auto micros_begin = clock::micros();
        iq16 tm = 0;

        for(size_t i = 0; i < times; i++){
            static constexpr iq16 dt = iq16(1) / isr_freq;
            tm += dt;
    
            raw_angle = Angle<uq32>::from_turns(frac(ac_freq * tm));
            // raw_angle = iq16(TAU) * frac((ac_freq-4.2_r) * tm);
            u0 = 32.0_r * raw_angle.sin() * (0.05_r * sin(8 * tm) + 1);
            spll.update(u0);
        }

        const auto micros_end = clock::micros();
        DEBUG_PRINT(iq16((micros_end - micros_begin).count()) / times,"us per call");
        DEBUG_PRINTLN(micros_begin, micros_end);
        while(true);
    }

    Microseconds dm = 0us;

    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
            case hal::TimerEvent::Update:{
                const auto m = clock::micros();
                run_sogi();
                dm = clock::micros() - m;
                break;
            }
            default: break;
        }
    });

    while(true){
        // DEBUG_PRINTLN_IDLE(raw_angle, spll.theta(), dm);
        DEBUG_PRINTLN(u0, raw_angle, spll.angle().to_degrees());
        clock::delay(1ms);
    }
}
void digipw_main(){
    DEBUGGER_INST.init({6_MHz});
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");

    /*-----------------------*/

    // test_sogi();
    auto scl_gpio = hal::PB<15>();
    auto sda_gpio = hal::PB<14>();
    
    hal::I2cSw i2csw{&scl_gpio, &sda_gpio};
    i2csw.init({1000000});
    
    // INA226 ina226{i2csw};
    // // ina226.init(10, 5);
    // ina226.init(100, 5);

    // auto & curr_ch = ina226.get_curr_channel();
    // auto & volt_ch = ina226.get_bus_volt_channel();
    // auto & power_ch = ina226.get_power_channel();

    /*-----------------------*/

    constexpr auto CHOPPER_FREQ = 100'000;
    auto & timer = hal::timer1;
    timer.init({
        .count_freq = hal::NearestFreq(CHOPPER_FREQ),
        .count_mode = hal::TimerCountMode::Up
    }, EN);
    timer.bdtr().init({10ns});

    auto & pwm = timer.oc<1>();
    auto & pwmn = timer.ocn<1>();

    pwm.init({});
    pwmn.init({});

    pwm.enable_cvr_sync(EN);
    auto en_gpio = hal::PB<0>();
    auto led = hal::PA<7>();

    en_gpio.outpp();
    led.outpp();

    // mp1907.enable();
    en_gpio.clr();

    /*-----------------------*/

    // BuckConverter buck{curr_ch, volt_ch, mp1907};
    // buck.init();

    // fixed_t<24> duty = 0.5_r;

    // size_t a;
    // DEBUG_PRINTLN(a);



    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
            case hal::TimerEvent::Update:{
                static iq20 mt = 0;
                static constexpr iq20 dt = 1_iq20 / CHOPPER_FREQ;
                mt += dt;
                // mp1907 = iq16(0.5) + 0.1_r * sinpu(50 * time());
                pwm.set_dutycycle(iq16(0.5) + 0.1_r * sinpu(50 * iq16(mt)));
                // const auto duty = 0.3_r;
                // mp1907 = CLAMP(duty, 0, 0.4_r);
                break;
            }
            default: break;
        }
    });

    while(true){
        
        // ina226.update();
        
        // const auto t = 6 * time();
        // const auto s = sinpu<31>(t);
        // DEBUG_PRINTLN_IDLE(iq16(curr_ch), iq16(volt_ch));
        // DEBUG_PRINTLN_IDLE(iq16(curr_ch), iq16(volt_ch), sin(t), sqrt(t), atan2(cos(t), sin(t)));

        // const auto curr_meas = fixed_t<24>(iq16(curr_ch));
        // const auto curr_targ = 0.04_q24;
        // const auto curr_err = curr_targ - curr_meas;
        // const auto delta = (curr_err * 0.007_r);
        // duty = duty + delta;
        // DEBUG_PRINTLN_IDLE(iq16(curr_ch), iq16(volt_ch), iq16(power_ch), duty, pwm.cvr());
        // DEBUG_PRINTLN_IDLE(iq16(curr_ch), iq16(volt_ch), iq16(power_ch));
        // DEBUG_PRINTLN_IDLE(iq16(volt_ch), iq16(curr_ch));

        // mp1907 = iq16(0.5) + iq16(0.4) * sin(t);

        //  + iq16(0.4) * sin(t);
        // mp1907 = duty;
        led.toggle();
    }
}
