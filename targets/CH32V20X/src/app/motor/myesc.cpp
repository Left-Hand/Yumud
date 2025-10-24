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

#include "drivers/Encoder/MagEnc/MT6825/mt6825.hpp"


#include "dsp/filter/firstorder/lpf.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"

#include "digipw/prelude/abdq.hpp"
#include "digipw/ctrl/pi_controller.hpp"
#include "drivers/GateDriver/uvw_pwmgen.hpp"


#include "dsp/motor_ctrl/sensorless/slide_mode_observer.hpp"
#include "dsp/motor_ctrl/sensorless/luenberger_observer.hpp"
#include "dsp/motor_ctrl/sensorless/nonlinear_flux_observer.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "hal/dma/dma.hpp"


using namespace ymd;

using namespace ymd::drivers;
using namespace ymd::digipw;
using namespace ymd::dsp;

#define DBG_UART hal::uart2

// static constexpr uint32_t DEBUG_UART_BAUD = 576000;

// #define TIM1_CH1_GPIO hal::PA<8>()
// #define TIM1_CH1N_GPIO hal::PA<7>()

// #define TIM1_CH2_GPIO hal::PA<9>()
// #define TIM1_CH2N_GPIO hal::PB<0>()

// #define TIM1_CH3_GPIO hal::PA<10>()
// #define TIM1_CH3N_GPIO hal::PB<1>()

// #define TIM1_CH4_GPIO hal::PA<11>()

// static constexpr uint32_t CHOPPER_FREQ = 24_KHz;
static constexpr uint32_t CHOPPER_FREQ = 48_KHz;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;

// static constexpr auto phase_inductance = 0.00275_q20;
// static constexpr auto phase_resistance = 10_q20;
static constexpr auto INV_BUS_VOLT = q16(1 / 12.0);
static constexpr size_t HFI_FREQ = 1000;

#if 1
static constexpr size_t POLE_PAIRS = 10u;
// static constexpr auto PHASE_INDUCTANCE = 0.0085_q20;
// static constexpr auto PHASE_INDUCTANCE = 0.00245_q20;
// static constexpr auto PHASE_INDUCTANCE = 0.0025_q20;

//100uh
static constexpr auto PHASE_INDUCTANCE = 0.0001_q20;

//1ohm
static constexpr auto PHASE_RESISTANCE = 1.123_q20;
#else
static constexpr size_t POLE_PAIRS = 7u;
// static constexpr auto PHASE_INDUCTANCE = 0.0085_q20;
// static constexpr auto PHASE_INDUCTANCE = 0.00245_q20;
// static constexpr auto PHASE_INDUCTANCE = 0.0025_q20;
static constexpr auto PHASE_INDUCTANCE = 0.0007_q20;
// static constexpr auto PHASE_INDUCTANCE = 0.00325_q20;
static constexpr auto PHASE_RESISTANCE = 0.523_q20;
#endif

static constexpr uint32_t CURRENT_CUTOFF_FREQ = 1000;
// static constexpr uint32_t CURRENT_CUTOFF_FREQ = 100;
static constexpr auto MAX_MODU_VOLT = q16(6.5);

namespace ymd::digipw{

//antiwinded pi controller
//kp + ki / s

struct [[nodiscard]] PiController {
    struct [[nodiscard]] Cofficients { 
        q16 kp;                // 比例系数
        q16 ki_discrete;       // 离散化积分系数（Ki * Ts）
        q16 max_out;          // 最大输出电压限制
        q16 err_sum_max;       // 积分项最大限制（抗饱和）

        constexpr PiController to_controller() const {
            return PiController(*this);
        }
        friend OutputStream & operator << (OutputStream & os, const Cofficients & self){
            return os << self.kp << 
                os.splitter() << self.ki_discrete 
                << os.splitter() << self.max_out 
                << os.splitter() << self.err_sum_max 
            ;
        }
    };

    using intergal_t = q20;

    constexpr PiController(const Cofficients& cfg):
        kp_(cfg.kp),
        ki_discrete_(cfg.ki_discrete),
        max_out_(cfg.max_out),
        err_sum_max_(MIN(static_cast<intergal_t>(cfg.err_sum_max), std::numeric_limits<intergal_t>::max()))
    {}

    constexpr void reset(){
        err_sum_ = 0;
    }

    constexpr auto operator()(const q20 err) {
        // q16 output = CLAMP2( + ki_discrete_ * q16(err_sum_), max_out_);
        const auto kp_contribute = CLAMP2(kp_ * q16(err), 0.1_q16);
        // const auto kp_contribute = kp_ * q16(err);
        q16 output = CLAMP2(kp_contribute + ki_discrete_ * q16(err_sum_), max_out_);
        err_sum_ = CLAMP(err_sum_ + err, -err_sum_max_ - output , err_sum_max_ - output);
        // err_sum_ = CLAMP(err_sum_ + err, -err_sum_max_, err_sum_max_);
        return output;
    }

public:
    q16 kp_;                // 比例系数
    q16 ki_discrete_;       // 离散化积分系数（Ki * Ts）
    q16 max_out_;          // 最大输出电压限制
    intergal_t err_sum_max_;       // 积分项最大限制（抗饱和）
    intergal_t err_sum_;           // 误差积分累加器
};

}


struct LrSeriesCurrentRegulatorConfig{
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 截止频率/带宽 (Hz)
    q16 phase_inductance;        // 相电感 (H)
    q16 phase_resistance;        // 相电阻 (Ω)
    q16 max_voltage;                // 最大电压 (V)

    [[nodiscard]] constexpr digipw::PiController::Cofficients make_coeff() const {
        //U(s) = I(s) * R + s * I(s) * L
        //I(s) / U(s) = 1 / (R + sL)
        //G_open(s) = (Ki / s + Kp) / s(R / s + L)

        // Ki = 2pi * fc * R
        // Kp = 2pi * fc * L

        const auto & self = *this;
        digipw::PiController::Cofficients coeff;

        const auto norm_freq = q16(q16(TAU) * fc / self.fs);
        coeff.max_out = self.max_voltage;

        coeff.kp = q20(self.phase_inductance * self.fc) * q16(TAU);
        // coeff.kp = 0;
        // coeff.kp = 1;

        coeff.ki_discrete = self.phase_resistance * norm_freq;

        // coeff.ki_discrete = 0;

        coeff.err_sum_max = self.max_voltage / q16(coeff.ki_discrete);
        return coeff;
    }
};

[[maybe_unused]] auto get_position_from_sine_curve(const q16 ctime){
    return Angle<q16>::from_turns(
        // ctime + sin(ctime)
        sin(ctime)
    );
};

[[maybe_unused]] auto get_position_from_linear_curve(){
    return Angle<q16>::from_turns(
        // ctime * 5
        // ctime * 1
        0
    );
};

[[maybe_unused]] auto forward_alphabeta_volt_by_sine_hfi = [](const q16 ctime){

    // return AlphaBetaCoord<q20>{.alpha = 0.0_q20, .beta = 1.5_q20};
    return AlphaBetaCoord<q20>{
        // .alpha = 1.0_q20 * sinpu(ctime * HFI_FREQ), 
        .alpha = 1.0_q20 * cospu(ctime * 2), 
        .beta = 1.0_q20 * sinpu(ctime * 2)
    };
};

#if 0
[[maybe_unused]] auto speed_compansate_dq_volt = [&]{
    auto dq_volt = dq_volt_;
    dq_volt.d = 0.0_q20;
    // dq_volt.d = 0.0005_q20 * linear_speed;
    // dq_volt.d = 0.0005_q20 * linear_speed;
    dq_volt.q = 0.0_q20;
    return dq_volt;
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
#endif


static void init_adc(){

    hal::adc1.init({
            {hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T28_5}
        },{

            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T7_5},
            {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T7_5},
            {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T7_5},

        },
        {}
    );

    hal::adc1.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    hal::adc1.enable_auto_inject(DISEN);
}


void myesc_main(){
    DBG_UART.init({
        .baudrate = DEBUG_UART_BAUD,
        .rx_strategy = CommStrategy::Dma,
        .tx_strategy = CommStrategy::Dma
    });
    DEBUGGER.retarget(&DBG_UART);
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

    auto & timer = hal::timer1;

    timer.init({
        .count_freq = hal::NearestFreq(CHOPPER_FREQ * 2), 
        .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
    }, EN);

    static constexpr auto MOS_1C840L_500MA_BEST_DEADZONE = 350ns;
    // static constexpr auto MOS_1C840L_100MA_BEST_DEADZONE = 350ns;
    timer.init_bdtr(MOS_1C840L_500MA_BEST_DEADZONE);
    // timer.init_bdtr(MOS_1C840L_100MA_BEST_DEADZONE);
    timer.set_remap(1);

    auto & pwm_u_ = timer.oc<1>(); 
    auto & pwm_v_ = timer.oc<2>(); 
    auto & pwm_w_ = timer.oc<3>(); 

    timer.ocn<1>().init({}); 
    timer.ocn<2>().init({}); 
    timer.ocn<3>().init({}); 

    hal::PA<7>().afpp();
    hal::PB<0>().afpp();
    hal::PB<1>().afpp();

    timer.oc<4>().cvr() = timer.arr() - 1;
    
    pwm_u_.init({});
    pwm_v_.init({});
    pwm_w_.init({});

    timer.oc<4>().init({
        .plant_en = DISEN
    });

    timer.oc<4>().enable_output(EN);

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


    // drv8323_idrive_gpio_.outpp(HIGH);//Sink 2A / Source1A
    drv8323_idrive_gpio_.inflt();
    // drv8323_idrive_gpio_.outpp(LOW);


    drv8323_vds_gpio_.outpp(LOW);
    // drv8323_vds_gpio_.outpp(HIGH); //dangerous no ocp protect!!!!

    auto blink_service_poller = [&]{

        led_red_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        led_blue_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
        led_green_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 800) > 400);
    };

    init_adc();

    auto soa_ = hal::ScaledAnalogInput(hal::adc1.inj<1>(), 
        Rescaler<q16>::from_anti_offset(1.65_r)     * Rescaler<q16>::from_scale(1.0_r));
    auto sob_ = hal::ScaledAnalogInput(hal::adc1.inj<2>(), 
        Rescaler<q16>::from_anti_offset(1.65_r)     * Rescaler<q16>::from_scale(1.0_r));
    auto soc_ = hal::ScaledAnalogInput(hal::adc1.inj<3>(), 
        Rescaler<q16>::from_anti_offset(1.65_r)    * Rescaler<q16>::from_scale(1.0_r));


    auto uvw_pwmgen_ = UvwPwmgen(&pwm_u_, &pwm_v_, &pwm_w_);

    Angle<q16> openloop_elecrad_ = 0_deg;
    UvwCoord<q20> uvw_curr_ = Zero;
    DqCoord<q20> dq_curr_ = Zero;
    DqCoord<q20> dq_volt_ = Zero;
    AlphaBetaCoord<q20> alphabeta_curr_ = Zero;
    AlphaBetaCoord<q20> alphabeta_volt_ = Zero;

    auto nfault_gpio_ = hal::PA<6>();
    nfault_gpio_.inpu();



    static constexpr auto current_regulator_cfg = LrSeriesCurrentRegulatorConfig{
        .fs = FOC_FREQ,
        .fc = CURRENT_CUTOFF_FREQ,
        .phase_inductance = PHASE_INDUCTANCE,
        .phase_resistance = PHASE_RESISTANCE,
        .max_voltage = MAX_MODU_VOLT,
    };

    static constexpr auto controller_coeff = current_regulator_cfg.make_coeff();
    // PANIC{controller_coeff};
    auto d_pi_ctrl_ = controller_coeff.to_controller();
    auto q_pi_ctrl_ = controller_coeff.to_controller();

    [[maybe_unused]] auto flux_sensorless_ob = dsp::motor_ctl::NonlinearFluxObserver{
        dsp::motor_ctl::NonlinearFluxObserver::Config{
            .fs = FOC_FREQ,
            .phase_inductance = PHASE_INDUCTANCE,
            .phase_resistance = PHASE_RESISTANCE,

            // .observer_gain = 0.16_q20, // [rad/s]
            .observer_gain = 0.1201_q20, // [rad/s]
            // .pm_flux_linkage = 0.000017_q20, // [V / (rad/s)]
            .pm_flux_linkage = 0.0003_q20, // [V / (rad/s)]

    }};

    [[maybe_unused]] auto lbg_sensorless_ob = dsp::motor_ctl::LuenbergerObserver{
        dsp::motor_ctl::LuenbergerObserver::Config{
            .fs = FOC_FREQ,
            .phase_inductance = PHASE_INDUCTANCE,
            .phase_resistance = PHASE_RESISTANCE
    }};

    [[maybe_unused]] auto smo_sensorless_ob = dsp::motor_ctl::SlideModeObserver{
        dsp::motor_ctl::SlideModeObserver::Config{
            .f_para = 0.84_r,
            .g_para = 0.015_r,
            .kslide = 1.22_r,   
            .kslf = 0.6_r,   
        }
    };

    Microseconds exe_us_ = 0us;

    auto ctrl_isr = [&]{
        const auto uvw_curr = UvwCoord<q20>{
            .u = soa_.get_value(),
            .v = sob_.get_value(),
            .w = soc_.get_value(),
        };

        const auto alphabeta_curr = AlphaBetaCoord<q20>::from_uvw(uvw_curr_);


        [[maybe_unused]] const auto ctime = clock::time();

        // const auto openloop_manchine_angle = Angle<q16>::from_turns(10_r * ctime);
        const auto openloop_manchine_angle = Angle<q16>::from_turns(0.2_r * ctime);
        // const auto openloop_manchine_angle = Angle<q16>::from_turns(0);
        const auto openloop_elec_angle = openloop_manchine_angle * POLE_PAIRS;
        // const auto elec_angle = openloop_elec_angle;

        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle()) - 10_deg;
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle()) - 20_deg;
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle()) - 40_deg;
        const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() - 90_deg);
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() + 80_deg);
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle()) - 40_deg;
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() + 180_deg + 30_deg);
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() + 50_deg);
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() - 22_deg);
        // const auto elec_angle = Angle<q16>(flux_sensorless_ob.angle() - 135_deg);
        // const auto elec_angle = Angle<q16>(lbg_sensorless_ob.angle() + 30_deg);
        // const auto elec_angle = Angle<q16>(smo_sensorless_ob.angle() + 90_deg);
        // const auto elec_angle = Angle<q16>(smo_sensorless_ob.angle() + 90_deg);
        // const auto elec_angle = Angle<q16>(smo_sensorless_ob.angle() + 90_deg);

        const auto elec_rotation = Rotation2<q16>::from_angle(elec_angle);
        const auto dq_curr = alphabeta_curr.to_dq(elec_rotation);

        [[maybe_unused]] auto forward_dq_volt_by_pi_ctrl = [&]{
            const auto dest_q_curr = CLAMP(4 * ctime - 1, 0, 0.4_r);
            // const auto dest_q_curr = 0.4_r + 0.2_q16 * sinpu(50 * ctime);
            // const auto dest_q_curr = 0.4_r;
            // const auto dest_q_curr = 0.4_q16;
            // const auto dest_q_curr = 0.26_q16;
            // const auto dest_q_curr = 0.14_q16;
            // const auto dest_q_curr = 0.02_q16;

            return DqCoord<q20>{
                .d = d_pi_ctrl_(0 - dq_curr.d),
                .q = q_pi_ctrl_(dest_q_curr - dq_curr.q)
            };
        };

        #if 0
        [[maybe_unused]] auto forward_dq_volt_by_constant_voltage = [&]{
            auto dq_volt = dq_volt_;
            dq_volt.d = 0_r;
            dq_volt.q = 0.2_r;
            return dq_volt;
        };
        #endif



        // const auto dq_volt = (forward_dq_volt_by_hfi()).clamp(MAX_MODU_VOLT);
        const auto dq_volt = (forward_dq_volt_by_pi_ctrl()).clamp(MAX_MODU_VOLT);
        // const auto dq_volt = forward_dq_volt_by_constant_voltage().clamp(MAX_MODU_VOLT);

        [[maybe_unused]] auto forward_alpha_beta_volt_by_hfi = [&]{
            static constexpr size_t HFI_MAX_STEPS = 4;
            static constexpr auto HFI_MAX_VOLT = 1.0_r;
            static size_t hfi_step = 0;
            hfi_step = (hfi_step + 1) % HFI_MAX_STEPS;
            
            return AlphaBetaCoord<q20>{
                .alpha = HFI_MAX_VOLT * sinpu(q16(hfi_step) / (HFI_MAX_STEPS)),
                // .alpha = HFI_MAX_VOLT,
                .beta = 0_r,
            };
        };


        const auto alphabeta_volt = dq_volt.to_alphabeta(elec_rotation);
        // const auto alphabeta_volt = AlphaBetaCoord<q20>::ZERO;
        // const auto alphabeta_volt = forward_alpha_beta_volt_by_hfi().clamp(MAX_MODU_VOLT);

        flux_sensorless_ob.update(alphabeta_volt, alphabeta_curr);
        // smo_sensorless_ob.update(alphabeta_volt, alphabeta_curr);

        const auto uvw_dutycycle = SVM(
            AlphaBetaCoord<q16>{
                .alpha = alphabeta_volt.alpha, 
                .beta = alphabeta_volt.beta
            } * INV_BUS_VOLT
        );

        uvw_pwmgen_.set_dutycycle(uvw_dutycycle);

        uvw_curr_ = uvw_curr;
        alphabeta_curr_ = alphabeta_curr;
        dq_curr_ = dq_curr;
        dq_volt_ = dq_volt;
        alphabeta_volt_ = alphabeta_volt;
        openloop_elecrad_ = openloop_elec_angle;
    };



    auto jeoc_isr = [&]{ 
        const auto begin_us = clock::micros();
        ctrl_isr();
        const auto end_us = clock::micros();
        exe_us_ = end_us - begin_us;
    };

    hal::adc1.register_nvic({0,0}, EN);
    hal::adc1.enable_interrupt<hal::AdcIT::JEOC>(EN);
    hal::adc1.set_event_callback(
        [&](const hal::AdcEvent ev){
            switch(ev){
            case hal::AdcEvent::EndOfInjectedConversion:{
                jeoc_isr();
                break;
            }
            default: break;
            }
        }
    );

    [[maybe_unused]] auto repl_service_poller = [&]{
        static robots::ReplServer repl_server{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",

            rpc::make_function("errn", [&](int32_t a, int32_t b){ 
                DEBUG_PRINTLN(a,b);
            }),
            rpc::make_function("errn2", [&](int32_t a, int32_t b){ 
                DEBUG_PRINTLN(a,b);
            })

        );

        repl_server.invoke(list);
    };

    while(true){
        // repl_service_poller();
        if(1) DEBUG_PRINTLN_IDLE(
            alphabeta_curr_,
            // alphabeta_volt_,
            // alphabeta_volt_.beta / alphabeta_curr_.beta,
            dq_curr_,
            dq_volt_,
            // q_pi_ctrl_.err_sum_,
            // q_pi_ctrl_.kp_
            // q_pi_ctrl_.err_sum_max_
            // lbg_sensorless_ob.angle().to_turns(),
            flux_sensorless_ob.angle().to_turns()
            // flux_sensorless_ob.V_alphabeta_last_
            // smo_sensorless_ob.angle().to_turns(),
            // exe_us_.count(),
            // SVM({.alpha = 0.1_q16, .beta = 0})

            // flux_sensorless_ob.angle().to_turns(),
            // lbg_sensorless_ob.angle().to_turns(),
            // smo_sensorless_ob.angle().to_turns(),
            // uint32_t(exe_us_.count())

            // openloop_elecrad_.normalized().to_turns()
            // bool(nfault_gpio_.read() == LOW),

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

        blink_service_poller();
        // repl_service_poller();
        // clock::delay(2ms);
    }

}