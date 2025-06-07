#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/fp/matchit.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"
#include "hal/gpio/gpio.hpp"

#include "drivers/Encoder/odometer.hpp"

#include "algo/interpolation/cubic.hpp"

#include "robots/rpc/rpc.hpp"
#include "src/testbench/tb.h"

#include "robots/rpc/arg_parser.hpp"

#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "core/polymorphism/reflect.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "drivers/Encoder/Encoder.hpp"
#include "drivers/Encoder/AnalogEncoder.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"

using namespace ymd;

#define UART hal::uart1

//AT8222
class StepperSVPWM{
public:

    hal::GenericTimer & timer_;

    static void init_channel(hal::TimerOC & oc){
        oc.init({.valid_level = LOW});
    }

    void init(const uint32_t freq){
        timer_.init({freq});
    }
private:
    static constexpr std::pair<real_t, real_t> split_duty_in_pair(const real_t duty){
        if(duty > 0){
            return {real_t(0), frac(duty)};
        }else{
            return {frac(-duty), real_t(0)};
        }
    }
};

// static constexpr size_t CHOP_FREQ = 30_KHz;
static constexpr size_t CHOP_FREQ = 20_KHz;
// static constexpr size_t CHOP_FREQ = 100;


void mystepper_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);


    // hal::Gpio & ena_gpio = hal::PB<0>;
    // hal::Gpio & enb_gpio = hal::PA<7>;
    hal::Gpio & ena_gpio = hal::portB[0];
    hal::Gpio & enb_gpio = hal::portA[7];
    auto & timer = hal::timer1;
    auto & pwm_ap = timer.oc<1>();
    auto & pwm_an = timer.oc<2>();
    auto & pwm_bp = timer.oc<3>();
    auto & pwm_bn = timer.oc<4>();

    ena_gpio.outpp(HIGH);
    enb_gpio.outpp(HIGH);


    timer.init({
        .freq = CHOP_FREQ,
        .mode = hal::TimerCountMode::CenterAlignedDualTrig
    });

    timer.enable_arr_sync();


    timer.set_trgo_source(hal::TimerTrgoSource::Update);

    const hal::TimerOcPwmConfig pwm_noinv_cfg = {
        .cvr_sync_en = EN,
        .valid_level = HIGH
    };

    const hal::TimerOcPwmConfig pwm_inv_cfg = {
        .cvr_sync_en = EN,
        .valid_level = LOW,
    };
    
    pwm_ap.init(pwm_noinv_cfg);
    pwm_an.init(pwm_noinv_cfg);
    pwm_bp.init(pwm_inv_cfg);
    pwm_bn.init(pwm_inv_cfg);

    hal::TimerOcPair pwm_a = {pwm_ap, pwm_an};
    hal::TimerOcPair pwm_b = {pwm_bp, pwm_bn};

    pwm_a.inverse(EN);
    pwm_b.inverse(DISEN);

    auto & adc = hal::adc1;
    adc.init(
        {
            {hal::AdcChannelIndex::VREF, hal::AdcSampleCycles::T28_5}
        },{
            {hal::AdcChannelIndex::CH3, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelIndex::CH2, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelIndex::CH3, hal::AdcSampleCycles::T28_5},
        }
    );

    adc.set_injected_trigger(hal::AdcOnChip::InjectedTrigger::T1TRGO);
    // adc.enable_auto_inject(DISEN);
    // adc.enable_continous(EN);
    adc.enable_auto_inject(EN);

    auto & inj_a = adc.inj<1>();
    auto & inj_b = adc.inj<3>();

    auto & trig_gpio = hal::PC<13>();
    trig_gpio.outpp();

    real_t a_curr;
    real_t b_curr;
    adc.attach(hal::AdcIT::JEOC, {0,0}, [&]{
        // trig_gpio.toggle();
        // DEBUG_PRINTLN_IDLE(millis());
        // trig_gpio.toggle();
        static bool is_a = false;
        is_a = !is_a;
        if(is_a){
            // DEBUG_PRINTLN_IDLE(a_curr);
            b_curr = inj_b.get_voltage();
            a_curr = inj_a.get_voltage();
        }else{
            b_curr = inj_b.get_voltage();
            a_curr = inj_a.get_voltage();
            // DEBUG_PRINTLN_IDLE(b_curr);
        }
    });

    timer.attach(hal::TimerIT::Update, {0,0}, [&](){
        // trig_gpio.toggle();
    });
    


    while(true){
        
        const auto t = clock::time();
        // const auto [st, ct] = sincospu(t * 93);
        // const auto [st, ct] = sincospu(t * 3);
        // const auto [st, ct] = sincospu(10 * sinpu(t));
        const auto [st, ct] = sincospu(120 * t);
        // const auto [st, ct] = sincospu(sinpu(t));
        const auto mag = 0.6_r;

        pwm_a.set_duty(st * mag);
        pwm_b.set_duty(ct * mag);

        DEBUG_PRINTLN_IDLE(
            // timer.oc<1>().cvr(), 
            // timer.oc<2>().cvr(), 
            // timer.oc<3>().cvr(), 
            // timer.oc<4>().cvr(), 

            // hal::portA[8].read().to_bool(),
            // hal::portA[9].read().to_bool(),
            // hal::portA[10].read().to_bool(),
            // hal::portA[11].read().to_bool(),

            // ena_gpio.read().to_bool(),
            // enb_gpio.read().to_bool(),
            // inj_a.get_voltage(),
            // inj_b.get_voltage(),
            ADC1->IDATAR1,
            ADC1->IDATAR2,
            ADC1->IDATAR3,
            a_curr, b_curr,
            trig_gpio.read().to_bool()
        );
    }
}