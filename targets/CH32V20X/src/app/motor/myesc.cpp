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


#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"

#include "digipw/prelude/abdq.hpp"
#include "drivers/GateDriver/uvw_pwmgen.hpp"


#include "dsp/motor_ctrl/sensorless/slide_mode_observer.hpp"
#include "dsp/motor_ctrl/sensorless/luenberger_observer.hpp"
#include "dsp/motor_ctrl/sensorless/nonlinear_flux_observer.hpp"

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
static constexpr uint32_t CHOPPER_FREQ = 32_KHz;


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

            {AdcChannelNth::CH1, AdcSampleCycles::T7_5},
            {AdcChannelNth::CH4, AdcSampleCycles::T7_5},
            {AdcChannelNth::CH5, AdcSampleCycles::T7_5},

            // {AdcChannelNth::CH1, AdcSampleCycles::T13_5},
            // {AdcChannelNth::CH4, AdcSampleCycles::T13_5},
            // {AdcChannelNth::CH5, AdcSampleCycles::T13_5},

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

    static constexpr auto MOS_1C840L_500MA_BEST_DEADZONE = 90ns;
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
        Rescaler<q16>::from_anti_offset(1.65_r)    * Rescaler<q16>::from_scale(1.0_r));


    auto uvw_pwmgen_ = UvwPwmgen(&pwm_u_, &pwm_v_, &pwm_w_);

    Angle<q16> openloop_elecrad_ = 0_deg;
    UvwCoord<q20> uvw_curr_ = {0};
    DqCoord<q20> dq_curr_ = {0};
    AlphaBetaCoord<q20> alpha_beta_curr_ = {0};
    AlphaBetaCoord<q20> alpha_beta_volt_ = {0};

    auto & nfault_gpio = hal::PA<6>();
    nfault_gpio.inpu();

    auto sensorless_ob = dsp::motor_ctl::LuenbergerObserver{{}};
    // auto sensorless_ob = dsp::motor_ctl::SlideModeObserver{{}};
    auto gen_wave_hfi = [&]{
        static constexpr q16 HALF_ONE = 0.5_q16;
        static constexpr q16 WAVE_AMP = 0.14_q16;

        static uint32_t cnt = 0;
        cnt++;

        const auto wave = WAVE_AMP * ((cnt & 0b01) ? 1 : -1);
        pwm_u_.set_dutycycle(HALF_ONE + wave);
        pwm_v_.set_dutycycle(HALF_ONE);
        pwm_w_.set_dutycycle(HALF_ONE);
    };



    auto dq_volt_ = DqCoord<q20>{
        .d = 0,
        .q = 0
    };

    auto ctrl_isr = [&]{
        uvw_curr_ = {
            .u = soa_.get_value(),
            .v = sob_.get_value(),
            .w = soc_.get_value(),
        };




        // gen_wave_hfi();
        // return;


        // const auto p = ctime * 80;
        // const auto p = 60 * sinpu(ctime/4);
        // const auto p = 60 * ctime;
        // const auto [s,c] = sincos(ctime * 80);
        [[maybe_unused]] const auto ctime = clock::time();
        // const auto [s,c] = sincos(ctime * 1.2_r);
        // const auto [s,c] = sincos(ctime * 20.2_r);
        // const auto [s,c] = sincos(0.0_r);
        // const auto [s,c] = sincos(p);
        // const auto mag = 0.06_r;
        static constexpr size_t POLE_PAIRS = 7u;
        [[maybe_unused]] auto get_position_from_sine_curve = [&]{
            return Angle<q16>::from_turns(
                // ctime + sin(ctime)
                sin(ctime)
            );
        };

        [[maybe_unused]] auto get_position_from_linear_curve = [&]{
            return Angle<q16>::from_turns(
                ctime * 2
            );
        };

        const auto openloop_position = get_position_from_linear_curve();
        const auto openloop_elecrad = openloop_position * POLE_PAIRS;
        // const auto openloop_elecrad = Angle<q20>::from_turns(0.5_r);


        const auto elecrad = openloop_elecrad;
        // const auto elecrad = Angle<q16>(sensorless_ob.angle()) - 10_deg;
        // const auto elecrad = Angle<q16>(sensorless_ob.angle()) - 10_deg;
        // const auto elecrad = Angle<q16>(sensorless_ob.angle()) + 10_deg;
        // const auto elecrad = Angle<q16>(sensorless_ob.angle() - 15_deg);
        alpha_beta_curr_ = AlphaBetaCoord<q20>::from_uvw(uvw_curr_);
        dq_curr_ = alpha_beta_curr_.to_dq(elecrad);


        static constexpr auto MAX_MODU_VOLT = q16(5.5);

        [[maybe_unused]] auto forward_dq_volt_by_pi_ctrl = [&]{
            auto dq_volt = dq_volt_;

            // const auto dest_q_volt = sinpu(ctime * 50.8_r) * 0.05_r + 0.25_r;
            // const auto dest_q_volt = sinpu(ctime * 50.8_r) * 0.05_r + 0.25_r;

            const auto dest_q_curr = 0.25_r;
            dq_volt.d = dq_volt.d + 0.004_q20 * (0 - dq_curr_.d);
            dq_volt.q = dq_volt.q + 0.004_q20 * (dest_q_curr - dq_curr_.q);
            return dq_volt;
        };

        [[maybe_unused]] auto forward_dq_volt_by_constant_voltage = [&]{
            auto dq_volt = dq_volt_;
            dq_volt.d = 2;
            dq_volt.q = 0;
            return dq_volt;
        };

        dq_volt_ = forward_dq_volt_by_pi_ctrl().clamp(MAX_MODU_VOLT);
        // dq_volt_ = forward_dq_volt_by_constant_voltage().clamp(MAX_MODU_VOLT);


        static constexpr auto INV_BUS_VOLT = q16(1 / 12.0);

        // const auto MODU_VOLT = 2.05_r;
        // const auto MODU_VOLT = 5.25_r;
        // const auto MODU_VOLT = 3.5_r + 1 * sinpu(ctime * 0.2_r);
        // const auto MODU_VOLT = ABS(3.4_r * sinpu(ctime * 0.2_r));


        const auto alpha_beta_volt = dq_volt_.to_alpha_beta(elecrad);



        sensorless_ob.update(alpha_beta_volt, alpha_beta_curr_);

        const auto uvw_dutycycle = SVM(
            AlphaBetaCoord<q16>{
                .alpha = alpha_beta_volt.alpha, 
                .beta = alpha_beta_volt.beta
            } * INV_BUS_VOLT);
        uvw_pwmgen_.set_dutycycle(uvw_dutycycle);

        alpha_beta_volt_ = alpha_beta_volt;
        openloop_elecrad_ = openloop_elecrad;
    };

    
    adc1.attach(hal::AdcIT::JEOC, {0,0}, 
        ctrl_isr, EN
    );



    while(true){
        DEBUG_PRINTLN_IDLE(
            // uvw_curr_,
            alpha_beta_curr_,
            dq_curr_,
            uvw_curr_,
            // dq_volt_,
            // alpha_beta_volt_,

            sensorless_ob.angle().to_turns()

            // openloop_elecrad_.normalized().to_turns()
            // bool(nfault_gpio.read() == LOW),

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
        );

        blink_service();
        // clock::delay(2ms);
    }

}