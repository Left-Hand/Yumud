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
#include "hal/dma/dma.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"
#include "digipw/ctrl/pi_controller.hpp"

#include "drivers/GateDriver/DRV832X/DRV832X.hpp"
#include "drivers/Encoder/MagEnc/MT6825/mt6825.hpp"
#include "drivers/GateDriver/uvw_pwmgen.hpp"


#include "dsp/motor_ctrl/sensorless/slide_mode_observer.hpp"
#include "dsp/motor_ctrl/sensorless/luenberger_observer.hpp"
#include "dsp/motor_ctrl/sensorless/nonlinear_flux_observer.hpp"
#include "dsp/controller/adrc/nltd2o.hpp"
#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/filter/firstorder/lpf.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"

#include "middlewares/rpc/rpc.hpp"
#include "middlewares/repl/repl_service.hpp"


#include "linear_regression.hpp"


//电机参数：
// https://item.taobao.com/item.htm?id=643573104607
using namespace ymd;

using namespace ymd::drivers;
using namespace ymd::digipw;
using namespace ymd::dsp;

#define DBG_UART hal::uart2

// static constexpr uint32_t DEBUG_UART_BAUD = 576000;
// static constexpr uint32_t CHOPPER_FREQ = 24_KHz;
static constexpr uint32_t CHOPPER_FREQ = 32_KHz;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;

// static constexpr auto phase_inductance = 0.00275_iq20;
// static constexpr auto phase_resistance = 10_iq20;
static constexpr auto INV_BUS_VOLT = iq16(1 / 12.0);
static constexpr size_t HFI_FREQ = 1000;

#if 1
static constexpr size_t POLE_PAIRS = 10u;
// static constexpr auto PHASE_INDUCTANCE = 0.0085_iq20;
// static constexpr auto PHASE_INDUCTANCE = 0.00245_iq20;
// static constexpr auto PHASE_INDUCTANCE = 0.0025_iq20;

//100uh
static constexpr auto PHASE_INDUCTANCE = iq20(42.3 * 1E-6);

//1ohm
// static constexpr auto PHASE_RESISTANCE = 1.123_iq20;
static constexpr auto PHASE_RESISTANCE = 0.823_iq20;
#else
static constexpr size_t POLE_PAIRS = 7u;
static constexpr auto PHASE_INDUCTANCE = 0.0007_iq20;
// static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
static constexpr auto PHASE_RESISTANCE = 0.523_iq20;
#endif

static constexpr uint32_t CURRENT_CUTOFF_FREQ = 1200;
// static constexpr uint32_t CURRENT_CUTOFF_FREQ = 200;

static constexpr auto MAX_MODU_VOLT = iq16(6.5);

struct LrSeriesCurrentRegulatorConfig{
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 截止频率/带宽 (Hz)
    iq16 phase_inductance;        // 相电感 (H)
    iq16 phase_resistance;        // 相电阻 (Ω)
    iq16 max_voltage;                // 最大电压 (V)

    [[nodiscard]] constexpr Result<digipw::PiController::Cofficients, const char *> try_to_coeff() const {
        //U(s) = I(s) * R + s * I(s) * L
        //I(s) / U(s) = 1 / (R + sL)
        //G_open(s) = (Ki / s + Kp) / s(R / s + L)

        // Ki = 2pi * fc * R
        // Kp = 2pi * fc * L

        const auto & self = *this;
        digipw::PiController::Cofficients coeff;

        const auto norm_omega = iq16(iq16(TAU) * fc / self.fs);
        coeff.max_out = self.max_voltage;

        coeff.kp = iq20(self.phase_inductance * self.fc) * iq16(TAU);
        coeff.ki_discrete = self.phase_resistance * norm_omega;

        // coeff.ki_discrete = 0;

        coeff.err_sum_max = self.max_voltage / iq16(coeff.ki_discrete);
        return Ok(coeff);
    }
};


#if 0
[[maybe_unused]] auto speed_compansate_dq_volt = [&]{
    auto dq_volt = dq_volt_;
    dq_volt.d = 0.0_iq20;
    // dq_volt.d = 0.0005_iq20 * linear_speed;
    // dq_volt.d = 0.0005_iq20 * linear_speed;
    dq_volt.q = 0.0_iq20;
    return dq_volt;
};

#endif


static constexpr auto current_regulator_cfg = LrSeriesCurrentRegulatorConfig{
    .fs = FOC_FREQ,
    .fc = CURRENT_CUTOFF_FREQ,
    .phase_inductance = PHASE_INDUCTANCE,
    .phase_resistance = PHASE_RESISTANCE,
    .max_voltage = MAX_MODU_VOLT,
};




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


    auto & timer = hal::timer1;

    // #region 初始化定时器

    timer.init({
        .count_freq = hal::NearestFreq(CHOPPER_FREQ * 2), 
        .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
    }, EN);

    static constexpr auto MOS_1C840L_500MA_BEST_DEADZONE = 90ns;
    // static constexpr auto MOS_1C840L_100MA_BEST_DEADZONE = 350ns;
    timer.bdtr().init({MOS_1C840L_500MA_BEST_DEADZONE});
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

    auto uvw_pwmgen_ = UvwPwmgen(&pwm_u_, &pwm_v_, &pwm_w_);

    // #endregion 初始化定时器

    // #region 配置编码器
    
    auto mt6825_cs_gpio_ = hal::PB<12>();
    mt6825_cs_gpio_.outpp();
    auto & spi = hal::spi2;

    spi.init({
        .baudrate = 18_MHz
    });

    drivers::MT6825 mt6825_{
        &spi,
        spi.allocate_cs_gpio(&mt6825_cs_gpio_)
            .unwrap()
    };


    // #endregion

    // #region 初始化DRV8323


    auto drv8323_en_gpio_ = hal::PA<11>();
    auto drv8323_slp_gpio_ = hal::PA<12>();
    auto drv8323_nfault_gpio_ = hal::PA<6>();
    drv8323_nfault_gpio_.inpu();

    drv8323_en_gpio_.outpp(LOW);
    drv8323_slp_gpio_.outpp(LOW);

    auto drv8323_mode_gpio_      = hal::PB<4>();
    auto drv8323_vds_gpio_       = hal::PB<3>();
    auto drv8323_idrive_gpio_    = hal::PB<5>();
    auto drv8323_gain_gpio_      = hal::PA<15>();


    drv8323_mode_gpio_.outpp(LOW);      //6x pwm
    // drv8323_mode_gpio_.outpp(HIGH);    //independent

    // drv8323_gain_gpio_.outpp(LOW);
    // drv8323_gain_gpio_.outpp(LOW);
    // drv8323_gain_gpio_.inpd();//10x
    drv8323_gain_gpio_.inflt();//20x
    // drv8323_gain_gpio_.outpp(HIGH);//40x


    drv8323_idrive_gpio_.outpp(HIGH);//Sink 2A / Source1A
    // drv8323_idrive_gpio_.inflt();
    // drv8323_idrive_gpio_.outpp(LOW);


    drv8323_vds_gpio_.outpp(LOW); //10A保护
    // drv8323_vds_gpio_.outpp(HIGH); //dangerous no ocp protect!!!!


    // #endregion 初始化ADC

    // #region 初始化ADC
    static constexpr auto VOLTAGE_TO_CURRENT_RATIO = 0.5_r;
    auto soa_ = hal::ScaledAnalogInput(hal::adc1.inj<1>(), 
        Rescaler<iq16>::from_anti_offset(1.65_r)     * Rescaler<iq16>::from_scale(VOLTAGE_TO_CURRENT_RATIO ));
    auto sob_ = hal::ScaledAnalogInput(hal::adc1.inj<2>(), 
        Rescaler<iq16>::from_anti_offset(1.65_r)     * Rescaler<iq16>::from_scale(VOLTAGE_TO_CURRENT_RATIO ));
    auto soc_ = hal::ScaledAnalogInput(hal::adc1.inj<3>(), 
        Rescaler<iq16>::from_anti_offset(1.65_r)    * Rescaler<iq16>::from_scale(VOLTAGE_TO_CURRENT_RATIO ));

    init_adc();
    // #endregion 

    // #region 初始化LED
    auto led_blue_gpio_ = hal::PC<13>();
    auto led_red_gpio_ = hal::PC<14>();
    auto led_green_gpio_ = hal::PC<15>();

    led_red_gpio_.outpp(); 
    led_blue_gpio_.outpp(); 
    led_green_gpio_.outpp();

    auto blink_service_poller = [&]{

        led_red_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        led_blue_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
        led_green_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 800) > 400);
    };

    // #endregion 
    
    Angle<uq16> openloop_elec_angle_ = Zero;
    Angle<uq16> sensored_elec_angle_ = Zero;
    Angle<uq16> encoder_lap_angle_ = Zero;
    Angle<iq16> encoder_multi_angle_ = Zero;
    Angle<iq16> diff_angle = Zero;
    UvwCoord<iq20> uvw_curr_ = Zero;
    DqCoord<iq20> dq_curr_ = Zero;
    DqCoord<iq20> dq_volt_ = Zero;
    AlphaBetaCoord<iq20> alphabeta_curr_ = Zero;
    AlphaBetaCoord<iq20> alphabeta_volt_ = Zero;
    Microseconds exe_us_ = 0us;



    static constexpr auto controller_coeff = current_regulator_cfg.try_to_coeff().unwrap();
    // PANIC{controller_coeff};

    auto d_pi_ctrl_ = controller_coeff.to_pi_controller();
    auto q_pi_ctrl_ = controller_coeff.to_pi_controller();

    [[maybe_unused]] auto flux_sensorless_ob = dsp::motor_ctl::NonlinearFluxObserver{
        dsp::motor_ctl::NonlinearFluxObserver::Config{
            .fs = FOC_FREQ,
            .phase_inductance = PHASE_INDUCTANCE,
            .phase_resistance = PHASE_RESISTANCE,

            // .observer_gain = 0.16_iq20, // [rad/s]
            .observer_gain = 0.1201_iq20, // [rad/s]
            // .pm_flux_linkage = 0.000017_iq20, // [V / (rad/s)]
            .pm_flux_linkage = 0.00084_iq20, // [V / (rad/s)]
        }
    };

    [[maybe_unused]] auto lbg_sensorless_ob = dsp::motor_ctl::LuenbergerObserver{
        dsp::motor_ctl::LuenbergerObserver::Config{
            .fs = FOC_FREQ,
            .phase_inductance = PHASE_INDUCTANCE,
            .phase_resistance = PHASE_RESISTANCE
        }
    };

    [[maybe_unused]] auto smo_sensorless_ob = dsp::motor_ctl::SlideModeObserver{
        dsp::motor_ctl::SlideModeObserver::Config{
            .f_para = 0.84_r,
            .g_para = 0.015_r,
            .kslide = 1.22_r,   
            .kslf = 0.6_r,   
        }
    };

    static constexpr auto coeffs = typename NonlinearTrackingDifferentor::Config{
        .fs = FOC_FREQ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        // .r = 152.5_iq10,
        .r = 242.5_iq10,
        .h = 0.005_iq10,
        .x2_limit = 240
    }.try_to_coeffs().unwrap();

    static NonlinearTrackingDifferentor command_shaper_{
        coeffs
    };

    SecondOrderState track_ref_;
    SecondOrderState feedback_state_;

    static constexpr auto feedback_coeffs = LinearTrackingDifferentiator<iq16, 2>::Config{
        .fs = FOC_FREQ, .r = 350
    }.try_to_coeffs().unwrap();

    [[maybe_unused]] LinearTrackingDifferentiator<iq16, 2> feedback_differ_{
        feedback_coeffs
    };
    
    auto ctrl_isr = [&]{
        [[maybe_unused]] const auto ctime = clock::time();

        //#region 电流传感
        const auto uvw_curr = UvwCoord<iq20>{
            .u = soa_.get_value(),
            .v = sob_.get_value(),
            .w = soc_.get_value(),
        };

        const auto alphabeta_curr = AlphaBetaCoord<iq20>::from_uvw(uvw_curr_);
        //#endregion

        //#region 位置提取
        const auto openloop_manchine_angle = Angle<iq16>::from_turns(0 * ctime);
        // const auto openloop_manchine_angle = Angle<iq16>::from_turns(1.2_r * ctime);
        // const auto openloop_manchine_angle = Angle<iq16>::from_turns(sinpu(0.2_r * ctime));
        const auto openloop_elec_angle = openloop_manchine_angle * POLE_PAIRS;

        static constexpr auto ANGLE_BASE = Angle<uq16>::from_turns(0.78_uq16);
        const auto next_encoder_lap_angle = mt6825_.get_lap_angle().examine().cast_inner<uq16>();

        // const auto diff_angle = (next_encoder_lap_angle.cast_inner<iq16>() 
        diff_angle = (next_encoder_lap_angle.cast_inner<iq16>() 
            - encoder_lap_angle_.cast_inner<iq16>()).normalized();

        encoder_lap_angle_ = next_encoder_lap_angle;
        encoder_multi_angle_ = encoder_multi_angle_ + diff_angle;
        feedback_state_ = feedback_differ_.update(feedback_state_, encoder_multi_angle_.to_turns());
        
        const auto sensored_elec_angle = ((next_encoder_lap_angle * POLE_PAIRS) + ANGLE_BASE).normalized(); 


        #if 1
        // const auto elec_angle = openloop_elec_angle;
        const auto elec_angle = sensored_elec_angle;
        #else
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle()) - 10_deg;
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle()) - 20_deg;
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle()) - 40_deg;
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle() - 90_deg);
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle() + 80_deg);
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle()) - 40_deg;
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle() + 180_deg + 30_deg);
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle() + 50_deg);
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle() - 22_deg);
        // const auto elec_angle = Angle<iq16>(flux_sensorless_ob.angle() - 135_deg);
        // const auto elec_angle = Angle<iq16>(lbg_sensorless_ob.angle() + 30_deg);
        // const auto elec_angle = Angle<iq16>(smo_sensorless_ob.angle() + 90_deg);
        // const auto elec_angle = Angle<iq16>(smo_sensorless_ob.angle() + 90_deg);
        // const auto elec_angle = Angle<iq16>(smo_sensorless_ob.angle() + 90_deg);
        #endif

        const auto elec_rotation = Rotation2<iq16>::from_angle(elec_angle);
        //#endregion

        //#region 位速合成力矩
        const auto [position_cmd, speed_cmd] = [&]{
            if constexpr(1){
                // command_shaper_.update(10 + 12 * sign(iq16(sinpu(ctime * 0.5_r))));
                // const auto s = iq16(sinpu(ctime * 0.7_r));
                // const auto s = iq16(sinpu(ctime * 0.16_r));
                // command_shaper_.update(100 + 6 * (int(s * 8) / 8));
                track_ref_ = command_shaper_.update(track_ref_, floor(ctime * 3) * 4, 0);
                return std::make_tuple(
                    iq16::from_bits(track_ref_.x1.to_bits() >> 16),
                    track_ref_.x2
                );
            }
            const auto omega = 9_iq16;
            const auto amplitude = 2_iq16;

            const auto [s,c] = sincos(omega * ctime);
            return std::make_tuple<iq16, iq16>(
                amplitude * s,
                amplitude * omega * c
            );
            // return std::make_tuple<iq16, iq16>(
            //     amplitude * int(omega * ctime),
            //     0
            // );
        }();


        const iq20 torque_cmd = [&]{ 
            const iq16 kp = 0.23_iq16;
            const iq16 kd = 0.035_iq16;

            // const iq16 position_err = position_cmd - pos_filter_.accumulated_angle().to_turns();
            // const iq16 speed_err = speed_cmd - pos_filter_.speed();
            const iq16 position_err = position_cmd - iq16::from_bits(feedback_state_.x1.to_bits() >> 16);
            const iq16 speed_err = speed_cmd - feedback_state_.x2;

            return (kp * position_err) + (kd * speed_err);
        }();

        //#endregion 

        //#region 力矩转电流

        static constexpr iq20 TORQUE_2_CURR_RATIO = 1_iq16;
        static constexpr iq20 MAX_CURRENT = 0.4_iq16;

        const iq20 current_cmd = CLAMP2(torque_cmd * TORQUE_2_CURR_RATIO, MAX_CURRENT);
        //#endregion

        const auto dq_curr = alphabeta_curr.to_dq(elec_rotation);

        [[maybe_unused]] auto generate_dq_volt_by_pi_ctrl = [&]{
            const iq20 dest_d_curr = 0;
            const iq20 dest_q_curr = current_cmd;
            return DqCoord<iq20>{
                .d = d_pi_ctrl_(dest_d_curr - dq_curr.d),
                .q = q_pi_ctrl_(dest_q_curr - dq_curr.q)
            };
        };

        #if 0
        [[maybe_unused]] auto generate_dq_volt_by_constant_voltage = [&]{
            auto dq_volt = dq_volt_;
            dq_volt.d = 0_r;
            dq_volt.q = 0.2_r;
            return dq_volt;
        };
        #endif



        // const auto dq_volt = (generate_dq_volt_by_hfi()).clamp(MAX_MODU_VOLT);
        const auto dq_volt = (generate_dq_volt_by_pi_ctrl()).clamp(MAX_MODU_VOLT);
        // const auto dq_volt = generate_dq_volt_by_constant_voltage().clamp(MAX_MODU_VOLT);

        #if 0
        [[maybe_unused]] auto generate_alpha_beta_volt_by_hfi = [&]{
            static constexpr size_t HFI_MAX_STEPS = 4;
            static constexpr auto HFI_MAX_VOLT = 1.0_r;
            static size_t hfi_step = 0;
            hfi_step = (hfi_step + 1) % HFI_MAX_STEPS;
            
            return AlphaBetaCoord<iq20>{
                .alpha = HFI_MAX_VOLT * sinpu(iq16(hfi_step) / (HFI_MAX_STEPS)),
                // .alpha = HFI_MAX_VOLT,
                .beta = 0_r,
            };
        };
        #endif


        const auto alphabeta_volt = dq_volt.to_alphabeta(elec_rotation);
        // const auto alphabeta_volt = AlphaBetaCoord<iq20>::ZERO;
        // const auto alphabeta_volt = generate_alpha_beta_volt_by_hfi().clamp(MAX_MODU_VOLT);

        // flux_sensorless_ob.update(alphabeta_volt, alphabeta_curr);
        // smo_sensorless_ob.update(alphabeta_volt, alphabeta_curr);

        const auto uvw_dutycycle = SVM(
            AlphaBetaCoord<iq16>{
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
        openloop_elec_angle_ = openloop_elec_angle;
        sensored_elec_angle_ = sensored_elec_angle;
    };



    auto jeoc_isr = [&]{ 
        const auto begin_us = clock::micros();
        ctrl_isr();
        const auto end_us = clock::micros();
        exe_us_ = end_us - begin_us;
    };

    hal::adc1.register_nvic({0,0}, EN);
    hal::adc1.enable_interrupt<hal::AdcIT::JEOC>(EN);
    hal::adc1.set_event_handler(
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
        if(DEBUGGER.pending() != 0) continue;

        DEBUG_PRINTLN(
            // alphabeta_curr_,
            // alphabeta_volt_,
            // alphabeta_volt_.beta / alphabeta_curr_.beta,
            // dq_curr_,
            // dq_volt_,
            // alphabeta_volt_.beta / alphabeta_curr_.beta,
            // q_pi_ctrl_.err_sum_,
            // q_pi_ctrl_.kp_
            // q_pi_ctrl_.err_sum_max_
            // lbg_sensorless_ob.angle().to_turns(),
            // hal::adc1.inj<1>().get_voltage(),
            
            // iq16(lap_angle.to_turns()) * POLE_PAIRS,
            // sensored_elec_angle_.to_turns(),
            // openloop_elec_angle_.to_turns(),
            iq16::from_bits(track_ref_.x1.to_bits() >> 16),
            track_ref_.x2,

            iq16::from_bits(feedback_state_.x1.to_bits() >> 16),
            feedback_state_.x2,
            // pos_filter_.accumulated_angle().to_turns(),
            // pos_filter_.speed(),
            // sensored_elec_angle_.to_turns(),
            encoder_multi_angle_.to_turns(),
            diff_angle.to_turns(),
            0
            // flux_sensorless_ob.angle().to_turns()
            // flux_sensorless_ob.V_alphabeta_last_
            // smo_sensorless_ob.angle().to_turns(),
            // exe_us_.count(),

            // flux_sensorless_ob.angle().to_turns(),
            // lbg_sensorless_ob.angle().to_turns(),
            // smo_sensorless_ob.angle().to_turns(),
            // uint32_t(exe_us_.count())

            // openloop_elec_angle_.normalized().to_turns()
            // bool(drv8323_nfault_gpio_.read() == LOW),


            // pwm_u_.cvr(),
            // pwm_v_.cvr(),
            // pwm_w_.cvr(),

            // pwm_u_.get_dutycycle(),
            // pwm_v_.get_dutycycle(),
            // pwm_w_.get_dutycycle(),
        );

        blink_service_poller();
        // repl_service_poller();
        // clock::delay(2ms);
    }

}