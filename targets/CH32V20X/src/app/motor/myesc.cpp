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




static constexpr int a = Zero; 

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

struct PiCurrentCtrl {
    struct FromIndResConfig {
        uint32_t fs;                 // 采样频率 (Hz)
        uint32_t fc;                 // 截止频率/带宽 (Hz)
        q20 phase_inductance;        // 相电感 (H)
        q20 phase_resistance;        // 相电阻 (Ω)
        q20 max_volt;                // 最大电压 (V)
    };

    static constexpr PiCurrentCtrl from_inductance_and_resistance(const FromIndResConfig & cfg){
        PiCurrentCtrl ret;
        ret.max_out_ = cfg.max_volt;
        q12 omega_bw = q12(TAU) * cfg.fc;
        ret.kp_ = cfg.phase_inductance * omega_bw;
        ret.ki_discrete_ = q12(cfg.phase_resistance) * omega_bw / cfg.fs;
        ret.err_sum_max_ = q16(cfg.max_volt * cfg.fs) / (q12(cfg.phase_resistance) * omega_bw);
        return ret;
    }

    constexpr void reset(){
        err_sum_ = q20(0);
    }

    constexpr auto operator()(const q20 err) {
        q20 output = CLAMP2(kp_ * err + err_sum_ * ki_discrete_, max_out_);
        err_sum_ = CLAMP(err_sum_ + err, -max_out_ - output , max_out_ - output);
        return output;
    }

    q20 kp_;                // 比例系数
    q24 ki_discrete_;       // 离散化积分系数（Ki * Ts）
    q20 max_out_;          // 最大输出电压限制
    q20 err_sum_max_;       // 积分项最大限制（抗饱和）
    q20 err_sum_;           // 误差积分累加器
};


void myesc_main(){
    DEBUG_UART.init({DEBUG_UART_BAUD});
    DEBUGGER.retarget(&DEBUG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    clock::delay(2ms);

    auto led_blue_gpio_ = hal::PC<13>();
    auto led_red_gpio_ = hal::PC<14>();
    auto led_green_gpio_ = hal::PC<15>();

    led_red_gpio_.outpp(); 
    led_blue_gpio_.outpp(); 
    led_green_gpio_.outpp();

    auto en_gpio = hal::PA<11>();
    auto slp_gpio = hal::PA<12>();

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

    auto drv8323_mode_gpio_      = hal::PB<4>();
    auto drv8323_vds_gpio_       = hal::PB<3>();
    auto drv8323_idrive_gpio_    = hal::PB<5>();
    auto drv8323_gain_gpio_      = hal::PA<15>();


    drv8323_mode_gpio_.outpp(LOW);      //6x pwm
    // drv8323_mode_gpio_.outpp(HIGH);    //independent

    // drv8323_gain_gpio_.outpp(LOW);
    // drv8323_gain_gpio_.outpp(LOW);
    drv8323_gain_gpio_.inpd();//20x
    // drv8323_gain_gpio_.outpp(HIGH);//40x


    drv8323_idrive_gpio_.outpp(HIGH);//Sink 2A / Source1A

    // drv8323_idrive_gpio_.inflt();
    // drv8323_idrive_gpio_.inpu();


    drv8323_vds_gpio_.outpp(LOW);

    //dangerous no ocp protect!!!!
    // drv8323_vds_gpio_.outpp(HIGH);

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

    auto nfault_gpio = hal::PA<6>();
    nfault_gpio.inpu();

    // static constexpr auto phase_inductance = 0.00275_q20;
    // static constexpr auto phase_resistance = 10_q20;

    static constexpr auto PHASE_INDUCTANCE = 0.00275_q20;
    static constexpr auto PHASE_RESISTANCE = 10_q20;
    // static constexpr uint32_t CURRENT_LOOP_BW = 1000;
    static constexpr uint32_t CURRENT_LOOP_BW = 1000;
    static constexpr auto MAX_MODU_VOLT = q16(6.5);

    auto d_pi_ctrl_ = PiCurrentCtrl::from_inductance_and_resistance({
            .fs = 32000,                 // 采样频率 (Hz)
            .fc = CURRENT_LOOP_BW,                 // 截止频率/带宽 (Hz)
            .phase_inductance = PHASE_INDUCTANCE,        // 相电感 (H)
            .phase_resistance = PHASE_RESISTANCE,        // 相电阻 (Ω)
            .max_volt = MAX_MODU_VOLT,                // 最大电压 (V)
        }
    );

    auto q_pi_ctrl_ = PiCurrentCtrl::from_inductance_and_resistance({
            .fs = 32000,                 // 采样频率 (Hz)
            .fc = CURRENT_LOOP_BW,                 // 截止频率/带宽 (Hz)
            .phase_inductance = PHASE_INDUCTANCE,        // 相电感 (H)
            .phase_resistance = PHASE_RESISTANCE,        // 相电阻 (Ω)
            .max_volt = MAX_MODU_VOLT,                // 最大电压 (V)
        }
    );

    auto flux_sensorless_ob = dsp::motor_ctl::NonlinearFluxObserver{
        dsp::motor_ctl::NonlinearFluxObserver::Config{
        .phase_inductance = PHASE_INDUCTANCE,
        .phase_resistance = PHASE_RESISTANCE,
        // .observer_gain = 0.05_q20, // [rad/s]
        .observer_gain = 0.06_q20, // [rad/s]
        .pm_flux_linkage = 0.27_q20, // [V / (rad/s)]
        // .pm_flux_linkage = 0.22_q20, // [V / (rad/s)]
        .freq = 32000,
    }};
    auto lbg_sensorless_ob = dsp::motor_ctl::LuenbergerObserver{{}};

    auto smo_sensorless_ob = dsp::motor_ctl::SlideModeObserver{
        dsp::motor_ctl::SlideModeObserver::Config{
            .f_para = 0.84_r,
            .g_para = 0.015_r,
            .kslide = 1.22_r,   
            .kslf = 0.6_r,   
        }
    };

    // auto gen_wave_hfi = [&]{
    //     static constexpr q16 HALF_ONE = 0.5_q16;
    //     static constexpr q16 WAVE_AMP = 0.14_q16;

    //     static uint32_t cnt = 0;
    //     cnt++;

    //     const auto wave = WAVE_AMP * ((cnt & 0b01) ? 1 : -1);
    //     pwm_u_.set_dutycycle(HALF_ONE + wave);
    //     pwm_v_.set_dutycycle(HALF_ONE);
    //     pwm_w_.set_dutycycle(HALF_ONE);
    // };



    DqCoord<q20> dq_volt_ = Zero;

    auto ctrl_isr = [&]{
        uvw_curr_ = {
            .u = soa_.get_value(),
            .v = sob_.get_value(),
            .w = soc_.get_value(),
        };
        alpha_beta_curr_ = AlphaBetaCoord<q20>::from_uvw(uvw_curr_);

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
                ctime * 1
                // 0
            );
        };






        const auto openloop_position = get_position_from_linear_curve();
        const auto openloop_elec_angle = openloop_position * POLE_PAIRS;
        // const auto openloop_elec_angle = Angle<q20>::from_turns(0.5_r);
        // const auto elec_angle = openloop_elec_angle;
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle()) - 10_deg;
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle()) - 40_deg;
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle()) - 70_deg;
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle()) + 10_deg;
        const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() + 180_deg + 70_deg);
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() + 50_deg);
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() - 135_deg);
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() - 135_deg);
        // const auto elec_angle = Angle<q16>(lbg_sensorless_ob.angle() + 30_deg);
        // const auto elec_angle = Angle<q16>(smo_sensorless_ob.angle() + 90_deg);
        dq_curr_ = alpha_beta_curr_.to_dq(elec_angle);




        [[maybe_unused]] auto forward_dq_volt_by_pi_ctrl = [&]{
            // auto dq_volt = dq_volt_;

            // const auto dest_q_volt = sinpu(ctime * 50.8_r) * 0.05_r + 0.25_r;
            const auto dest_q_curr = sinpu(ctime * 2.8_r) * 0.05_r + 0.25_r;
            // const auto dest_q_curr = 0.14_r * sinpu(ctime * 0.2_r);
            // const auto dest_q_curr = 0.1_r;

            // const auto dest_q_curr = 0.15_r + 0.1_r ;
            // dq_volt.d = dq_volt.d + 0.014_q20 * (0 - dq_curr_.d);
            // dq_volt.q = dq_volt.q + 0.014_q20 * (dest_q_curr - dq_curr_.q);
            return DqCoord<q20>{
                .d = d_pi_ctrl_(0 - dq_curr_.d),
                .q = q_pi_ctrl_(dest_q_curr - dq_curr_.q)
            };
            // return dq_volt;
        };

        [[maybe_unused]] auto forward_dq_volt_by_constant_voltage = [&]{
            auto dq_volt = dq_volt_;
            dq_volt.d = 0_r;
            dq_volt.q = 1.3_r;
            return dq_volt;
        };

        [[maybe_unused]] auto speed_compansate_dq_volt = [&]{
            auto dq_volt = dq_volt_;
            dq_volt.d = 0.0_q20;
            // dq_volt.d = 0.0005_q20 * linear_speed;
            // dq_volt.d = 0.0005_q20 * linear_speed;
            dq_volt.q = 0.0_q20;
            return dq_volt;
        };

        dq_volt_ = (forward_dq_volt_by_pi_ctrl()).clamp(MAX_MODU_VOLT);
        // dq_volt_ = forward_dq_volt_by_constant_voltage().clamp(MAX_MODU_VOLT);


        static constexpr auto INV_BUS_VOLT = q16(1 / 12.0);

        // const auto MODU_VOLT = 2.05_r;
        // const auto MODU_VOLT = 5.25_r;
        // const auto MODU_VOLT = 3.5_r + 1 * sinpu(ctime * 0.2_r);
        // const auto MODU_VOLT = ABS(3.4_r * sinpu(ctime * 0.2_r));

        [[maybe_unused]] auto forward_alpha_beta_volt_by_dq_volt = [&]{
            return (dq_volt_ + speed_compansate_dq_volt()).to_alphabeta(elec_angle);
        };
        
        // [[maybe_unused]] auto forward_alpha_beta_volt_by_constant_voltage = [&]{
        //     // return AlphaBetaCoord<q20>{.alpha = 0.0_q20, .beta = 1.5_q20};
        //     return AlphaBetaCoord<q20>{.alpha = 0.0_q20, .beta = 0.5_q20};
        // };

        [[maybe_unused]] auto forward_alpha_beta_volt_by_sine_hfi = [&]{
            [[maybe_unused]] static constexpr size_t HFI_FREQ = 1000;
            // return AlphaBetaCoord<q20>{.alpha = 0.0_q20, .beta = 1.5_q20};
            return AlphaBetaCoord<q20>{
                // .alpha = 1.0_q20 * sinpu(ctime * HFI_FREQ), 
                .alpha = 1.0_q20 * cospu(ctime * 2), 
                .beta = 1.0_q20 * sinpu(ctime * 2)
            };
        };


        const auto alpha_beta_volt = forward_alpha_beta_volt_by_dq_volt();
        // const auto alpha_beta_volt = forward_alpha_beta_volt_by_constant_voltage();
        // const auto alpha_beta_volt = forward_alpha_beta_volt_by_sine_hfi();



        flux_sensorless_ob.update(alpha_beta_volt, alpha_beta_curr_);
        lbg_sensorless_ob.update(alpha_beta_volt, alpha_beta_curr_);
        smo_sensorless_ob.update(alpha_beta_volt, alpha_beta_curr_);    

        const auto uvw_dutycycle = SVM(
            AlphaBetaCoord<q16>{
                .alpha = alpha_beta_volt.alpha, 
                .beta = alpha_beta_volt.beta
            } * INV_BUS_VOLT);
        uvw_pwmgen_.set_dutycycle(uvw_dutycycle);

        alpha_beta_volt_ = alpha_beta_volt;
        openloop_elecrad_ = openloop_elec_angle;
    };

    Microseconds exe_us_ = 0us;
    adc1.attach(hal::AdcIT::JEOC, {0,0}, 
        [&]{
            const auto begin_us = clock::micros();
            ctrl_isr();
            const auto end_us = clock::micros();
            exe_us_ = end_us - begin_us;
        }, EN
    );



    while(true){
        DEBUG_PRINTLN_IDLE(
            // uvw_curr_,
            alpha_beta_curr_,
            dq_curr_,
            dq_volt_,
            // uvw_curr_,

            // -0.02_q20 - uvw_curr_.u - uvw_curr_.v,
            // uvw_curr_.numeric_sum(),
            // dq_volt_,
            // alpha_beta_volt_,

            flux_sensorless_ob.angle().to_turns(),
            lbg_sensorless_ob.angle().to_turns(),
            smo_sensorless_ob.angle().to_turns(),
            uint32_t(exe_us_.count())

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