#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"

#include "drivers/Encoder/odometer.hpp"
#include "digipw/SVPWM/svpwm.hpp"
#include "digipw/SVPWM/svpwm3.hpp"
#include "drivers/GateDriver/DRV832X/DRV832X.hpp"

#include "dsp/observer/smo/SmoObserver.hpp"
#include "dsp/observer/lbg/RolbgObserver.hpp"
#include "dsp/observer/nonlinear/NonlinearObserver.hpp"

#include "core/polymorphism/traits.hpp"

#include "robots/rpc/rpc.hpp"
#include "src/testbench/tb.h"

#include "robots/rpc/arg_parser.hpp"

#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "core/polymorphism/reflect.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;
using namespace ymd::foc;
using namespace ymd::digipw;
using namespace ymd::dsp;

#define DEBUG_UART hal::uart2

// static constexpr uint32_t DEBUG_UART_BAUD = 576000;
static constexpr uint32_t CHOPPER_FREQ = 40_KHz;


static void init_adc(){

    adc1.init({
            {AdcChannelIndex::VREF, AdcSampleCycles::T28_5}
        },{
            // {AdcChannelIndex::CH5, AdcSampleCycles::T28_5},
            // {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
            // {AdcChannelIndex::CH1, AdcSampleCycles::T28_5},

            // {AdcChannelIndex::CH5, AdcSampleCycles::T7_5},
            // {AdcChannelIndex::CH4, AdcSampleCycles::T7_5},
            // {AdcChannelIndex::CH1, AdcSampleCycles::T7_5},
            // {AdcChannelIndex::VREF, AdcSampleCycles::T7_5},

            {AdcChannelIndex::CH5, AdcSampleCycles::T13_5},
            {AdcChannelIndex::CH4, AdcSampleCycles::T13_5},
            {AdcChannelIndex::CH1, AdcSampleCycles::T13_5},
            // {AdcChannelIndex::VREF, AdcSampleCycles::T7_5},
            // {AdcChannelIndex::TEMP, AdcSampleCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T41_5},
        }
    );

    // adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
    adc1.set_injected_trigger(AdcInjectedTrigger::T1CC4);
    // adc1.enableContinous();
    adc1.enable_auto_inject(DISEN);
}

template<size_t N, typename T>
std::array<T, N> ones(const T value){
    std::array<T, N> ret;
    ret.fill(value);
    return ret;
}
void myesc_main(){
    DEBUG_UART.init(DEBUG_UART_BAUD);
    DEBUGGER.retarget(&DEBUG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets();

    clock::delay(200ms);



    auto & en_gpio = portA[11];
    auto & slp_gpio = portA[12];

    en_gpio.outpp(LOW);
    slp_gpio.outpp(LOW);

    timer1.init(CHOPPER_FREQ, TimerCountMode::CenterAlignedUpTrig);

    static constexpr auto MOS_1C840L_500MA_BEST_DEADZONE = 150ns;
    // static constexpr auto MOS_1C840L_100MA_BEST_DEADZONE = 350ns;
    timer1.init_bdtr(MOS_1C840L_500MA_BEST_DEADZONE);
    // timer1.init_bdtr(MOS_1C840L_100MA_BEST_DEADZONE);
    timer1.remap(1);

    auto & pwm_u = timer1.oc<1>(); 
    auto & pwm_v = timer1.oc<2>(); 
    auto & pwm_w = timer1.oc<3>(); 

    pwm_u.init({.install_en = DISEN}); 
    pwm_v.init({.install_en = DISEN}); 
    pwm_w.init({.install_en = DISEN}); 
    
    portA[7].afpp();
    portB[0].afpp();
    portB[1].afpp();
        //     #define TIM1_CH1_GPIO portA[8]
        // #define TIM1_CH1N_GPIO portA[7]

        // #define TIM1_CH2_GPIO portA[9]
        // #define TIM1_CH2N_GPIO portB[0]

        // #define TIM1_CH3_GPIO portA[10]
        // #define TIM1_CH3N_GPIO portB[1]

        // #define TIM1_CH4_GPIO portA[11]
    timer1.oc<4>().cvr() = timer1.arr() - 1;
    
    pwm_u.init({});
    pwm_v.init({});
    pwm_w.init({});
    timer1.oc<4>().init({
        .install_en = DISEN
    });

    timer1.oc<4>().enable_output(EN);


    // drivers::DRV8323H  mosdrv{hal::SpiDrv{
    //     spi1, 
    //     spi1.attach_next_cs(portA[15]).value()
    // }};

    auto & mode_gpio = hal::portB[4];
    auto & vds_gpio = hal::portB[3];
    auto & idrive_gpio = hal::portB[5];
    auto & gain_gpio = hal::portA[15];

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


    init_adc();

    auto & soa = adc1.inj<1>();
    auto & sob = adc1.inj<2>();
    auto & soc = adc1.inj<3>();

    pwm_u.set_duty(0.06_r);

    real_t u_curr;
    real_t v_curr;
    real_t w_curr;

    auto & nfault_gpio = hal::PA<6>();
    nfault_gpio.inpu();

    timer1.attach(TimerIT::Update, {0,0}, [&]{
        u_curr = soa.get_voltage();
        v_curr = sob.get_voltage();
        w_curr = soc.get_voltage();
        // const auto t = clock::time();
        // const auto p = t * 80;
        // const auto p = 60 * sinpu(t/4);
        // const auto p = 60 * t;
        // const auto [s,c] = sincos(t * 80);
        // const auto [s,c] = sincos(p);
        // const auto mag = 0.02_r;
        // const auto [u, v, w] = SVM(s * mag, c * mag);
        const auto [u, v, w] = ones<3>(0.2_r);
        pwm_u.set_duty(u);
        pwm_v.set_duty(v);
        pwm_w.set_duty(w);


    });

    while(true){
        DEBUG_PRINTLN_IDLE(
            u_curr,
            v_curr,
            w_curr,
            bool(nfault_gpio.read() == LOW),
            // mosdrv.get_status1().unwrap().as_bitset(),
            // mosdrv.get_status2().unwrap().as_bitset(),

            0
        );


        // clock::delay(2ms);
    }

}