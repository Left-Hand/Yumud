#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/analog/opa/opa.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "drivers/GateDriver/DRV832X/DRV832X.hpp"

#include "dsp/motor_ctrl/sensorless/slide_mode_observer.hpp"
#include "dsp/motor_ctrl/sensorless/luenberger_observer.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;
using namespace ymd::digipw;
using namespace ymd::dsp;

#define DEBUG_UART hal::uart2

// static constexpr uint32_t DEBUG_UART_BAUD = 576000;

// #define TIM1_CH1_GPIO hal::PA<8>()
// #define TIM1_CH1N_GPIO hal::PA<7>()

// #define TIM1_CH2_GPIO hal::PA<9>()
// #define TIM1_CH2N_GPIO hal::PB<0>()

// #define TIM1_CH3_GPIO hal::PA<10>()
// #define TIM1_CH3N_GPIO hal::PB<1>()

// #define TIM1_CH4_GPIO hal::PA<11>()

// static constexpr uint32_t CHOPPER_FREQ = 20_KHz;
static constexpr uint32_t CHOPPER_FREQ = 20_KHz;


static void init_adc(){

    adc1.init({
            {AdcChannelNth::VREF, AdcSampleCycles::T28_5}
        },{
            // {AdcChannelNth::CH5, AdcSampleCycles::T28_5},
            // {AdcChannelNth::CH4, AdcSampleCycles::T28_5},
            // {AdcChannelNth::CH1, AdcSampleCycles::T28_5},

            // {AdcChannelNth::CH5, AdcSampleCycles::T7_5},
            // {AdcChannelNth::CH4, AdcSampleCycles::T7_5},
            // {AdcChannelNth::CH1, AdcSampleCycles::T7_5},
            // {AdcChannelNth::VREF, AdcSampleCycles::T7_5},

            {AdcChannelNth::CH1, AdcSampleCycles::T13_5},
            {AdcChannelNth::CH4, AdcSampleCycles::T13_5},
            {AdcChannelNth::CH5, AdcSampleCycles::T13_5},
            // {AdcChannelNth::VREF, AdcSampleCycles::T7_5},
            // {AdcChannelNth::TEMP, AdcSampleCycles::T7_5},
            // AdcChannelConfig{AdcChannelNth::CH1, AdcCycles::T7_5},
            // AdcChannelConfig{AdcChannelNth::CH4, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelNth::CH5, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelNth::CH1, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelNth::CH4, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelNth::CH5, AdcCycles::T41_5},
        },
        {}
    );

    adc1.set_injected_trigger(AdcInjectedTrigger::T1CC4);
    adc1.enable_auto_inject(DISEN);
}

template<size_t N, typename T>
std::array<T, N> ones(const T value){
    std::array<T, N> ret;
    ret.fill(value);
    return ret;
}


void myesc_main(){
    DEBUG_UART.init({DEBUG_UART_BAUD});
    DEBUGGER.retarget(&DEBUG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    clock::delay(2ms);

    auto & led_blue_gpio_ = hal::PC<13>();
    auto & led_red_gpio_ = hal::PC<14>();
    auto & led_green_gpio_ = hal::PC<15>();

    led_red_gpio_.outpp(); 
    led_blue_gpio_.outpp(); 
    led_green_gpio_.outpp();

    auto & en_gpio = hal::PA<11>();
    auto & slp_gpio = hal::PA<12>();

    en_gpio.outpp(LOW);
    slp_gpio.outpp(LOW);

    timer1.init({
        CHOPPER_FREQ, 
        TimerCountMode::CenterAlignedUpTrig
    }, EN);

    static constexpr auto MOS_1C840L_500MA_BEST_DEADZONE = 150ns;
    // static constexpr auto MOS_1C840L_100MA_BEST_DEADZONE = 350ns;
    timer1.init_bdtr(MOS_1C840L_500MA_BEST_DEADZONE);
    // timer1.init_bdtr(MOS_1C840L_100MA_BEST_DEADZONE);
    timer1.remap(1);

    auto & pwm_u_ = timer1.oc<1>(); 
    auto & pwm_v_ = timer1.oc<2>(); 
    auto & pwm_w_ = timer1.oc<3>(); 

    timer1.ocn<1>().init({}); 
    timer1.ocn<2>().init({}); 
    timer1.ocn<3>().init({}); 

    pwm_u_.init({.install_en = DISEN}); 
    pwm_v_.init({.install_en = DISEN}); 
    pwm_w_.init({.install_en = DISEN}); 
    
    hal::PA<7>().afpp();
    hal::PB<0>().afpp();
    hal::PB<1>().afpp();

    timer1.oc<4>().cvr() = timer1.arr() - 1;
    
    pwm_u_.init({});
    pwm_v_.init({});
    pwm_w_.init({});
    timer1.oc<4>().init({
        .install_en = DISEN
    });

    timer1.oc<4>().enable_output(EN);

    auto & mode_gpio = hal::PB<4>();
    auto & vds_gpio = hal::PB<3>();
    auto & idrive_gpio = hal::PB<5>();
    auto & gain_gpio = hal::PA<15>();

    //6xpwm
    // mode_gpio.outpp(LOW);
    
    //inde
    mode_gpio.outpp(HIGH);

    // gain_gpio.outpp(LOW);
    // gain_gpio.outpp(LOW);
    gain_gpio.inpd();


    idrive_gpio.outpp(HIGH);
    // idrive_gpio.inflt();
    // idrive_gpio.inpu();


    vds_gpio.outpp(LOW);

    //dangerous no ocp protect!!!!
    // vds_gpio.outpp(HIGH);

    // mosdrv.init({}).examine();

    auto blink_service = [&]{

        led_red_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 200) > 100);
        led_blue_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
        led_green_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 800) > 400);
    };

    init_adc();

    auto soa_ = ScaledAnalogInput(adc1.inj<1>(), 
        Rescaler<q16>::from_anti_offset(1.65_r)     * Rescaler<q16>::from_scale(1.0_r));
    auto sob_ = ScaledAnalogInput(adc1.inj<2>(), 
        Rescaler<q16>::from_anti_offset(1.65_r)     * Rescaler<q16>::from_scale(1.0_r));
    auto soc_ = ScaledAnalogInput(adc1.inj<3>(), 
        Rescaler<q16>::from_anti_offset(1.645_r)    * Rescaler<q16>::from_scale(1.0_r));

    pwm_u_.set_dutycycle(0.06_r);

    real_t u_curr_ = 0;
    real_t v_curr_ = 0;
    real_t w_curr_ = 0;

    auto & nfault_gpio = hal::PA<6>();
    nfault_gpio.inpu();

    auto    ctrl_isr = [&]{
        u_curr_ = soa_.get_value();
        v_curr_ = sob_.get_value();
        w_curr_ = soc_.get_value();


        // const auto p = ctime * 80;
        // const auto p = 60 * sinpu(ctime/4);
        // const auto p = 60 * ctime;
        // const auto [s,c] = sincos(ctime * 80);
        [[maybe_unused]] const auto ctime = clock::time();
        // const auto [s,c] = sincos(ctime * 1.2_r);
        // const auto [s,c] = sincos(ctime * 20.2_r);
        // const auto [s,c] = sincos(0.0_r);
        const auto [s,c] = sincospu(ctime * 3);
        // const auto [s,c] = sincos(p);
        // const auto mag = 0.06_r;
        const auto mag = 0.16_r;
        const auto [u, v, w] = SVM(c * mag, s * mag);
        // const auto [u, v, w] = ones<3>(0.2_r);
        pwm_u_.set_dutycycle(u);
        pwm_v_.set_dutycycle(v);
        pwm_w_.set_dutycycle(w);

    };

    
    adc1.attach(hal::AdcIT::JEOC, {0,0}, 
        ctrl_isr, EN
    );



    while(true){
        DEBUG_PRINTLN_IDLE(
            u_curr_,
            v_curr_,
            w_curr_,
            bool(nfault_gpio.read() == LOW),

            // hal::PA<8>().read().to_bool(),
            // hal::PA<9>().read().to_bool(),
            // hal::PA<10>().read().to_bool(),

            // pwm_u_.cvr(),
            // pwm_v_.cvr(),
            // pwm_w_.cvr(),

            // pwm_u_.get_dutycycle(),
            // pwm_v_.get_dutycycle(),
            // pwm_w_.get_dutycycle(),
            // mosdrv.get_status1().unwrap().as_bitset(),
            // mosdrv.get_status2().unwrap().as_bitset(),

            0
        );

        blink_service();
        clock::delay(2ms);
    }

}