#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/utils/default.hpp"
#include "core/async/timer.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/analog/opa/opa.hpp"
#include "hal/dma/dma.hpp"

#include "dsp/fft/fft32.hpp"
#include "dsp/motor_ctrl/sensorless/slide_mode_observer.hpp"
#include "dsp/motor_ctrl/sensorless/luenberger_observer.hpp"
#include "dsp/motor_ctrl/sensorless/nonlinear_flux_observer.hpp"
#include "dsp/controller/adrc/nonlinear/nltd2o.hpp"
#include "dsp/controller/adrc/linear/ltd2o.hpp"
#include "dsp/filter/firstorder/lpf.hpp"
#include "dsp/filter/butterworth/band.hpp"

#include "middlewares/rpc/rpc.hpp"
#include "middlewares/rpc/repl_server.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"
#include "digipw/ctrl/pi_controller.hpp"


#include "linear_regression.hpp"
#include "motor_profiles.hpp"

#include "core/sdk.hpp"


#include "dsp_lpf.hpp"
#include "dsp_vec.hpp"

using namespace ymd;

using namespace ymd::drivers;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::dsp::adrc;
using namespace ymd::myesc;




#define DBG_UART hal::usart2

// static constexpr uint32_t DEBUG_UART_BAUD = 576000;
// static constexpr uint32_t CHOPPER_FREQ = 24_KHz;


// using MotorProfile = MotorProfile_Ysc;
// using MotorProfile = MotorProfile_Gim4010;
// using MotorProfile = MotorProfile_M06Bare;
// using MotorProfile = MotorProfile_3505;
using MotorProfile = MotorProfile_36BLDB;


struct LrSeriesCurrentRegulatorConfig{
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 截止频率/带宽 (Hz)
    iq20 phase_inductance;        // 相电感 (H)
    iq20 phase_resistance;        // 相电阻 (Ω)
    iq20 voltage_limit;                // 最大电压 (V)

    [[nodiscard]] constexpr Result<digipw::PiController::Cofficients, StringView> try_into_coeffs() const {
        //U(s) = I(s) * R + s * I(s) * L
        //I(s) / U(s) = 1 / (R + sL)
        //G_open(s) = (Ki / s + Kp) / s(R / s + L)

        // Ki = 2pi * fc * R
        // Kp = 2pi * fc * L

        if(fs >= 65535) return Err(StringView("fs too large"));
        if(fc * 8 >= fs) return Err(StringView("fc too large"));

        const auto & self = *this;
        digipw::PiController::Cofficients coeffs;

        const auto norm_omega = iq24::from_bits(static_cast<int32_t>((static_cast<int64_t>(iq24(TAU).to_bits()) * fc) / self.fs));
        coeffs.max_out = self.voltage_limit;

        coeffs.kp = iq20(self.phase_inductance * self.fc) * iq16(TAU);
        coeffs.ki_discrete = self.phase_resistance * norm_omega;

        coeffs.err_sum_max = self.voltage_limit / iq16(coeffs.ki_discrete);
        return Ok(coeffs);
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


template<size_t FC, size_t Q>
static constexpr math::fixed_t<Q, int32_t> lpf_specified_fc(
    const math::fixed_t<Q, int32_t> x_state, 
    const math::fixed_t<Q, int32_t> x_new
){
    constexpr auto ALPHA = dsp::calc_lpf_alpha_uq32(FOC_FREQ, FC).unwrap();
    return lpf_with_given_alpha(x_state, x_new, ALPHA);
}


template<size_t Q>
static constexpr math::fixed_t<Q, int32_t> lpf_10hz(
    math::fixed_t<Q, int32_t> x_state, 
    const math::fixed_t<Q, int32_t> x_new
){
    return lpf_specified_fc<10>(x_state, x_new);
}

template<size_t Q>
static constexpr math::fixed_t<Q, int32_t> lpf_100hz(math::fixed_t<Q, int32_t> x_state, const math::fixed_t<Q, int32_t> x_new){
    return lpf_specified_fc<100>(x_state, x_new);
}

template<size_t Q>
static constexpr math::fixed_t<Q, int32_t> lpf_1000hz(math::fixed_t<Q, int32_t> x_state, const math::fixed_t<Q, int32_t> x_new){
    return lpf_specified_fc<1000>(x_state, x_new);
}

template<size_t Q>
static constexpr math::fixed_t<Q, int32_t> lpf_allpass(math::fixed_t<Q, int32_t> x_state, const math::fixed_t<Q, int32_t> x_new){
    return x_new;
}


static constexpr auto CURRENT_REGULATOR_CFG = LrSeriesCurrentRegulatorConfig{
    .fs = FOC_FREQ,
    .fc = MotorProfile::CURRENT_CUTOFF_FREQ,
    .phase_inductance = MotorProfile::PHASE_INDUCTANCE,
    .phase_resistance = MotorProfile::PHASE_RESISTANCE,
    .voltage_limit = MotorProfile::MODU_VOLT_LIMIT,
};

static constexpr auto PI_CONTROLLER_COEFFS = CURRENT_REGULATOR_CFG.try_into_coeffs().unwrap();

static constexpr size_t HFI_N = 32;
static_assert(std::has_single_bit(HFI_N));

static void init_adc(){

    hal::adc1.init({
            {hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T28_5}
        },{

            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T13_5},
            // {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T13_5},
            // {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T13_5},

            // {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T7_5},
            // {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T7_5},
            // {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T7_5},

        },
        {}
    );

    hal::adc1.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    // hal::adc1.set_injected_trigger(hal::AdcInjectedTrigger::T1TRGO);
    hal::adc1.enable_auto_inject(DISEN);
}

using namespace ymd::math;


static constexpr iiq32 uq32_wrapped_diff(const uq32 last, const uq32 now){
    const iiq32 diff = iiq32::from_bits(
        static_cast<int64_t>(now.to_bits()) - static_cast<int64_t>(last.to_bits())
    );
    if(diff > iiq32(0.5)) return diff - 1;
    if(diff < iiq32(-0.5)) return diff + 1;
    return diff;
};

static_assert(uq32_wrapped_diff(0.35_uq32, 0.6_uq32).to_bits() == iiq32(0.25).to_bits());
static_assert(uq32_wrapped_diff(0.75_uq32, 0.50_uq32).to_bits() == iiq32(-0.25).to_bits());
static_assert(uq32_wrapped_diff(0.05_uq32, 0.80_uq32).to_bits() == iiq32(-0.25).to_bits());

static constexpr iiq32 iiq32_inc_uq32_wrapped(const iiq32 state, const uq32 last, const uq32 now){
    const auto diff = uq32_wrapped_diff(last, now);
    return state + diff;
}

void myesc_main(){
    DBG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        // .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        .baudrate = hal::NearestFreq(6000000),
        // .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking,
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);
    // DEBUGGER.force_sync(EN);


    clock::delay(2ms);
    auto & timer = hal::timer1;

    // #region 初始化定时器

    // static constexpr auto MOS_1C840L_500MA_BEST_DEADZONE = 90ns;
    static constexpr auto MOS_1C840L_500MA_BEST_DEADZONE = 200ns;
    // static constexpr auto MOS_1C840L_100MA_BEST_DEADZONE = 350ns;
    timer.bdtr().init({MOS_1C840L_500MA_BEST_DEADZONE});
    // timer.init_bdtr(MOS_1C840L_100MA_BEST_DEADZONE);

    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__A7_B0_B1,
        .count_freq = hal::NearestFreq(CHOPPER_FREQ * 2), 
        // .count_freq = hal::timer::ArrAndPsc{2880-1,1-1},
        // .count_mode = hal::TimerCountMode::CenterAlignedDualTrig,
        .count_mode = hal::TimerCountMode::CenterAlignedUpTrig,
        // .count_mode = hal::TimerCountMode::CenterAlignedDownTrig,
        // .count_mode = hal::TimerCountMode::Up
    })  .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
            hal::TimerChannelSelection::CH4,

            hal::TimerChannelSelection::CH1N,
            hal::TimerChannelSelection::CH2N,
            hal::TimerChannelSelection::CH3N,
        }).unwrap()
        ;

    // timer.enable_cc_ctrl_sync(EN);
    timer.enable_arr_sync(EN);
    // timer.set_trgo_source(hal::TimerTrgoSource::OC4R);
    timer.set_trgo_source(hal::TimerTrgoSource::Update);

    auto & pwm_u_ = timer.oc<1>(); 
    auto & pwm_v_ = timer.oc<2>(); 
    auto & pwm_w_ = timer.oc<3>(); 

    pwm_u_.init(Default);
    pwm_v_.init(Default);
    pwm_w_.init(Default);

    timer.ocn<1>().init(Default); 
    timer.ocn<2>().init(Default); 
    timer.ocn<3>().init(Default); 


    pwm_u_.enable_cvr_sync(EN);
    pwm_v_.enable_cvr_sync(EN);
    pwm_w_.enable_cvr_sync(EN);

    timer.oc<4>().init(Default);
    timer.oc<4>().enable_cvr_sync(DISEN);
    // timer.oc<4>().cvr() = timer.arr() - 10;
    timer.oc<4>().cvr() = 10;
    // timer.oc<4>().cvr() = timer.arr() / 2;

    timer.oc<4>().enable_output(EN);


    const auto full_arr = timer.arr();
    // constexpr auto full_arr = 7200;
    auto set_uvw_dutycycle = [&]<typename T>(const T & dutycycle){
        // timer.enable_udis(DISEN);
        pwm_u_.set_dutycycle((dutycycle.template get<0>()));
        pwm_v_.set_dutycycle((dutycycle.template get<1>()));
        pwm_w_.set_dutycycle((dutycycle.template get<2>()));
        // timer.enable_udis(EN);
    };

    auto stop_pwm = [&]{
        timer.stop();
    };

    // #endregion 初始化定时器

    // #region 初始化ADC
    static constexpr auto SHUNT_RES = 0.002f;
    static constexpr auto OPA_GAIN = 40;
    static constexpr auto AMPS_FULLSCALE = 3.3 / (OPA_GAIN * SHUNT_RES);
    static constexpr auto AMPS_PER_ADC_LSB = iq20(AMPS_FULLSCALE / (1 << 12));
    // const auto scaler_u = Rescaler<iq16>::from_scale(AMPS_PER_ADC_LSB);
    // const auto scaler_v = Rescaler<iq16>::from_scale(AMPS_PER_ADC_LSB);
    // const auto scaler_w = Rescaler<iq16>::from_scale(AMPS_PER_ADC_LSB);

    init_adc();

    auto & u_current_sense_ch_ = hal::adc1.inj<1>();
    auto & v_current_sense_ch_ = hal::adc1.inj<2>();
    auto & w_current_sense_ch_ = hal::adc1.inj<3>();

    auto get_uvw_current_bits = [&] -> std::tuple<uint16_t, uint16_t, uint16_t>{ 
        return std::make_tuple(
            u_current_sense_ch_.read_u12(),
            v_current_sense_ch_.read_u12(),
            w_current_sense_ch_.read_u12()
        );
    };

    stop_pwm();
    //确保pwm完全停止
    clock::delay(20ms);
    // #endregion 
    auto timming_watch_pin_ = hal::PA<12>();
    timming_watch_pin_.outpp();

    // #region 配置编码器
    
    auto mag_encoder_cs_pin_ = hal::PB<12>();
    mag_encoder_cs_pin_.outpp();


    auto & spi = hal::spi2;

    spi.init({
        .remap = hal::SpiRemap::_0,
        .baudrate = hal::NearestFreq(18_MHz)
    });

    auto mag_encoder_ = MotorProfile::MagEncoder{
        &spi,
        spi.allocate_cs_pin(&mag_encoder_cs_pin_)
            .unwrap()
    };


    // #endregion

    // #region 初始化DRV8323


    auto drv8323_en_pin_ = hal::PA<11>();
    auto drv8323_slp_pin_ = hal::PA<12>();
    auto drv8323_nfault_pin_ = hal::PA<6>();
    drv8323_nfault_pin_.inpu();

    drv8323_en_pin_.outpp(LOW);
    drv8323_slp_pin_.outpp(LOW);

    auto drv8323_mode_pin_      = hal::PB<4>();
    auto drv8323_vds_pin_       = hal::PB<3>();
    auto drv8323_idrive_pin_    = hal::PB<5>();
    auto drv8323_gain_pin_      = hal::PA<15>();




    drv8323_mode_pin_.outpp(LOW);      //6x pwm
    // drv8323_mode_pin_.outpp(HIGH);    //independent

    // drv8323_gain_pin_.outpp(LOW);
    // drv8323_gain_pin_.outpp(LOW);
    // drv8323_gain_pin_.inpd();//10x
    // drv8323_gain_pin_.inflt();//20x
    drv8323_gain_pin_.outpp(HIGH);//40x


    drv8323_idrive_pin_.outpp(HIGH);//Sink 2A / Source1A
    // drv8323_idrive_pin_.inflt();
    // drv8323_idrive_pin_.outpp(LOW);


    drv8323_vds_pin_.outpp(LOW); //10A保护
    // drv8323_vds_pin_.outpp(HIGH); //dangerous no ocp protect!!!!


    // #endregion 


    // #region 初始化LED
    auto led_blue_pin_ = hal::PC<13>();
    auto led_red_pin_ = hal::PC<14>();
    auto led_green_pin_ = hal::PC<15>();

    led_red_pin_.outpp(); 
    led_blue_pin_.outpp(); 
    led_green_pin_.outpp();

    [[maybe_unused]] auto poll_blink_service = [&]{

        led_red_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        led_blue_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
        led_green_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 800) > 400);
    };

    [[maybe_unused]] auto toggle_red_led = [&]{
        led_red_pin_.toggle();
    };

    [[maybe_unused]] auto toggle_blue_led = [&]{
        led_blue_pin_.toggle();
    };

    // #endregion 
    
    Angular<uq32> openloop_elec_angle_ = Zero;
    Angular<uq32> encoder_elec_angle_ = Zero;
    Angular<uq32> hfi_elec_angle_ = Zero;
    Angular<uq32> encoder_lap_angle_ = Zero;
    Angular<iiq32> encoder_multilap_angle_ = Zero;

    Angular<uq16> hfi_lap_angle_ = Zero;
    Angular<iq16> hfi_multilap_angle_ = Zero;

    UvwCoord<iq20> uvw_current_ = Zero;
    DqCoord<iq20> dq_current_ = Zero;
    DqCoord<iq20> dq_volt_ = Zero;
    AlphaBetaCoord<iq20> alphabeta_current_ = Zero;
    AlphaBetaCoord<iq20> hfi_alphabeta_volt_ = Zero;
    AlphaBetaCoord<iq20> alphabeta_volt_ = Zero;

    [[maybe_unused]] static constexpr iq20 HFI_VOLT_LIMIT = MotorProfile::PHASE_RESISTANCE * 1.4_iq20;
    // static constexpr iq20 HFI_VOLT = MIN((, 2);
    // static constexpr iq20 HFI_VOLT = 1;
    static constexpr iq20 HFI_VOLT = 0;
    static iq20 prev_sample = Zero;
    static size_t hfi_idx = 0;
    static std::array<iq20, HFI_N> hfi_buffer;
    static bool is_samp_n = false;
    Microseconds exe_elapsed_ = 0us;

    Leso leso = Leso{
        MotorProfile::leso_coeffs
    };

    Leso::State leso_state_var_ = Zero;

    // PANIC{PI_CONTROLLER_COEFFS};

    auto d_pi_ctrl_ = PI_CONTROLLER_COEFFS.to_pi_controller();
    auto q_pi_ctrl_ = PI_CONTROLLER_COEFFS.to_pi_controller();

    [[maybe_unused]] auto flux_sensorless_ob = dsp::motor_ctl::NonlinearFluxObserver{
        dsp::motor_ctl::NonlinearFluxObserver::Config{
            .fs = FOC_FREQ,
            .phase_inductance = MotorProfile::PHASE_INDUCTANCE,
            .phase_resistance = MotorProfile::PHASE_RESISTANCE,

            // .observer_gain = 0.16_iq20, // [rad/s]
            .observer_gain = 0.1201_iq20, // [rad/s]
            // .pm_flux_linkage = 0.000017_iq20, // [V / (rad/s)]
            .pm_flux_linkage = 0.00084_iq20, // [V / (rad/s)]
        }
    };

    [[maybe_unused]] auto lbg_sensorless_ob = dsp::motor_ctl::LuenbergerObserver{
        dsp::motor_ctl::LuenbergerObserver::Config{
            .fs = FOC_FREQ,
            .phase_inductance = MotorProfile::PHASE_INDUCTANCE,
            .phase_resistance = MotorProfile::PHASE_RESISTANCE
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

    using Nltd2o = NonlinearTrackingDifferentiator<iq16, 2>;

    #if 0
    static constexpr auto command_shaper_coeffs = Nltd2o::Config{
        .fs = FOC_FREQ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        // .r = 152.5_iq10,
        .r = 202.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 240
    }.try_into_coeffs().unwrap();
    #else
    static constexpr auto command_shaper_coeffs = Nltd2o::Config{
        .fs = FOC_FREQ,
        .r = 587.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 40
    }.try_into_coeffs().unwrap();

    #endif

    static Nltd2o command_shaper_{
        command_shaper_coeffs
    };

    SecondOrderState<iq16> track_ref_;
    SecondOrderState<iq16> rotor_rotation_state_var_;

    using Ltd2o = dsp::adrc::LinearTrackingDifferentiator<iq16, 2>;
    static constexpr auto rotor_rotation_ltd_coeffs = Ltd2o::Config{
        .fs = FOC_FREQ, .r = 500
    }.try_into_coeffs().unwrap();

    // PANIC{rotor_rotation_ltd_coeffs};

    [[maybe_unused]] Ltd2o rotor_rotation_ltd_{
        rotor_rotation_ltd_coeffs
    };

    UvwCoord<iq16> uvw_dutycycle_ = Zero;
    iq20 busbar_current_unfilted_ = Zero;
    iq20 busbar_current_ = Zero;
    iq20 hfi_response_real_bin0_ = Zero;
    iq20 hfi_response_real_bin1_ = Zero;
    iq20 hfi_response_imag_bin1_ = Zero;
    iq20 hfi_response_real_bin2_ = Zero;
    iq20 hfi_response_imag_bin2_ = Zero;
    Microseconds last_exe_us_ = 0us;
    Microseconds exe_duration_ = 0us;
    std::tuple<uint32_t, uint32_t, uint32_t> uvw_current_bits_offset_acc_ = {0, 0, 0};
    std::tuple<uint16_t, uint16_t, uint16_t> uvw_current_bits_offset_ = {0, 0, 0};

    static constexpr size_t DC_CAL_TIMES = 32 * 128;
    bool dc_cal_done_ = false;
    size_t dc_cal_cnt_ = 0;
    [[maybe_unused]] auto ctrl_isr = [&]{
        const auto uvw_current_bits = get_uvw_current_bits();
        if(dc_cal_done_ == false) [[unlikely]]{
            uvw_current_bits_offset_acc_ = std::make_tuple<uint32_t, uint32_t, uint32_t>(
                std::get<0>(uvw_current_bits_offset_acc_) + static_cast<uint32_t>(std::get<0>(uvw_current_bits)),
                std::get<1>(uvw_current_bits_offset_acc_) + static_cast<uint32_t>(std::get<1>(uvw_current_bits)),
                std::get<2>(uvw_current_bits_offset_acc_) + static_cast<uint32_t>(std::get<2>(uvw_current_bits))
            );
            dc_cal_cnt_++;
            if(dc_cal_cnt_ >= DC_CAL_TIMES){
                uvw_current_bits_offset_ = std::make_tuple<uint16_t, uint16_t, uint16_t>(
                    static_cast<uint16_t>(std::get<0>(uvw_current_bits_offset_acc_) / int(DC_CAL_TIMES)),
                    static_cast<uint16_t>(std::get<1>(uvw_current_bits_offset_acc_) / int(DC_CAL_TIMES)),
                    static_cast<uint16_t>(std::get<2>(uvw_current_bits_offset_acc_) / int(DC_CAL_TIMES))
                );
                dc_cal_done_ = true;
            }

            {
                const auto LOW_HFI_VOLT = iq20(0.6_iq16);
                const auto [s,c] = dsp::SINCOS32_1_TABLE[dc_cal_cnt_ & 0x1f];
                const auto alphabeta_volt = AlphaBetaCoord<iq16>{
                    .alpha = iq15::from_bits(s.to_bits()) * LOW_HFI_VOLT,
                    .beta = iq15::from_bits(c.to_bits()) * LOW_HFI_VOLT
                };
                
                set_uvw_dutycycle(SVM(alphabeta_volt * INV_BUS_VOLT * iq16(1.5)));
            }
            return;
        }


        //#region 电流传感

        const auto uvw_curr = UvwCoord<iq20>{
            .u = static_cast<int32_t>(std::get<0>(uvw_current_bits) - static_cast<int32_t>(std::get<0>(uvw_current_bits_offset_)))
                 * AMPS_PER_ADC_LSB,
            .v = static_cast<int32_t>(std::get<1>(uvw_current_bits) - static_cast<int32_t>(std::get<1>(uvw_current_bits_offset_)))
                 * AMPS_PER_ADC_LSB,
            .w = static_cast<int32_t>(std::get<2>(uvw_current_bits) - static_cast<int32_t>(std::get<2>(uvw_current_bits_offset_)))
                 * AMPS_PER_ADC_LSB,
        };

        const auto alphabeta_current_unfilted = AlphaBetaCoord<iq20>::from_uvw(uvw_current_);
        const auto alphabeta_curr = alphabeta_current_unfilted;
        // const auto alphabeta_curr = AlphaBetaCoord{
        //     lpf_with_given_alpha(alphabeta_current_.alpha, alphabeta_current_unfilted.alpha),
        //     lpf_with_given_alpha(alphabeta_current_.beta, alphabeta_current_unfilted.beta),
        // };
        //#endregion

        //#region 位置提取

        [[maybe_unused]] const auto now_secs = clock::seconds();
        [[maybe_unused]] static constexpr auto ZERO_ELEC_ANGLE = Angular<uq32>::ZERO;
        const auto hfi_angle = Angular<uq32>::from_turns(
            math::pu_to_uq32(math::atan2pu(hfi_response_imag_bin2_ - 0.002_iq20, hfi_response_real_bin2_))
        );
        // const auto openloop_manchine_multilap_angle = Angular<iq16>::from_turns(0.02_iq16 * now_secs);
        const auto openloop_manchine_multilap_angle = Angular<iq16>::from_turns(0.20_iq16 * now_secs);

        const auto openloop_elec_angle = make_angle_from_turns(
            uq32(frac(openloop_manchine_multilap_angle.to_turns()) * MotorProfile::POLE_PAIRS));

        // static constexpr bool HAS_MAG_ENCODER = false;
        // mag_encoder_.update().examine();
        Angular<uq32> encoder_elec_angle = Zero;
        Angular<uq32> hfi_elec_angle = Zero;
        if(false){
            const auto next_encoder_lapturns = mag_encoder_.read_lap_angle().examine().to_turns();

            const auto encoder_diff_angle = uq32_wrapped_diff(encoder_lap_angle_.to_turns(), next_encoder_lapturns);

            encoder_lap_angle_ = Angular<uq32>::from_turns(next_encoder_lapturns);
            encoder_multilap_angle_ = Angular<iiq32>::from_turns(encoder_multilap_angle_.to_turns() + encoder_diff_angle);
            rotor_rotation_state_var_ = rotor_rotation_ltd_.iterate(
                rotor_rotation_state_var_, 
                {fixed_downcast<16>(encoder_multilap_angle_.to_turns()), 0}
            );
            
            
            encoder_elec_angle = Angular<uq32>::from_turns(pu_to_uq32(next_encoder_lapturns * MotorProfile::POLE_PAIRS)
                    + MotorProfile::SENSORED_ELEC_ANGLE_BASE.to_turns()); 
        }else{

            const auto next_hfi_lap_angle = hfi_angle.cast_inner<uq16>();

            const auto hfi_diff_angle = (next_hfi_lap_angle.cast_inner<iq16>() 
                - hfi_lap_angle_.cast_inner<iq16>()).signed_normalized() / 2;

            hfi_lap_angle_ = next_hfi_lap_angle;
            hfi_multilap_angle_ = hfi_multilap_angle_ + hfi_diff_angle;
            rotor_rotation_state_var_ = rotor_rotation_ltd_.iterate(
                rotor_rotation_state_var_, {hfi_multilap_angle_.to_turns(), 0});
            
            
            hfi_elec_angle = make_angle_from_turns(uq32((hfi_multilap_angle_).unsigned_normalized().to_turns())); 
            hfi_elec_angle_ = hfi_elec_angle;
        }



        #if 1
        // const Angular<uq32> elec_angle = Angular<iq16>::ZERO;
        // const Angular<uq32> elec_angle = ZERO_ELEC_ANGLE;
        const Angular<uq32> elec_angle = Angular<uq32>::from_turns(0.5_uq16 * now_secs);
        // const Angular<uq32> elec_angle = hfi_elec_angle + Angular<uq16>::QUARTER;
        // const Angular<uq32> elec_angle = encoder_elec_angle;
        #else
        // const Angular<uq32> elec_angle = Angular<iq16>(flux_sensorless_ob.angle()) - 10_deg;
        #endif

        const auto elec_rotation = Rotation2<iq16>::from_angle(elec_angle);
        [[maybe_unused]] const auto elec_omega = rotor_rotation_state_var_.x2 * MotorProfile::POLE_PAIRS;
        //#endregion

        //#region 位速合成力矩
        const auto [position_cmd, speed_cmd] = [&]{

            enum class ExamplePattern{
                Sine,
                Saw,
                Levels
            };

            // static constexpr auto example_pattern = ExamplePattern::Saw;
            static constexpr auto example_pattern = ExamplePattern::Levels;

            const auto [x1_cmd, x2_cmd] = [&] -> std::tuple<iq16, iq16>{
                if constexpr(example_pattern == ExamplePattern::Sine){
                    constexpr auto omega = 16_iq16;
                    constexpr auto side_amplitude = 0.4_iq16;

                    const auto [s,c] = math::sincos(omega * now_secs);
                    return {
                        side_amplitude * iq16(s),
                        side_amplitude * omega * iq16(c)
                    };
                }else if constexpr(example_pattern == ExamplePattern::Saw){
                    // const auto [s,c] = math::sincos(omega * now_secs);

                    constexpr auto freq = 1.6_iq16;
                    constexpr auto amplitude = 0.8_iq16;
                    constexpr auto slew_rate = amplitude * freq;
                    return {math::frac(now_secs * freq) * amplitude, slew_rate};
                }else if constexpr(example_pattern == ExamplePattern::Levels){
                    constexpr auto freq = 0.3_iq16;
                    constexpr size_t num_steps = 6;
                    constexpr auto half_amplitude = 0.4_iq16;
                    constexpr auto step = half_amplitude * 2/ num_steps;
                    const auto s = iq16(math::sinpu(now_secs * freq));
                    return {(math::floor(s * (num_steps / 2)) * step), 0};
                }
            }();
            // command_shaper_.update(10 + 12 * sign(iq16(math::sinpu(now_secs * 0.5_r))));
            // const auto s = iq16(math::sinpu(now_secs * 0.7_r));
            // const auto s = iq16(math::sinpu(now_secs * 0.16_r));
            // command_shaper_.update(100 + 6 * (int(s * 8) / 8));
            // track_ref_ = command_shaper_.update(track_ref_, );
            track_ref_ = command_shaper_.iterate(track_ref_, {
                x1_cmd, 
                x2_cmd
            });
            // track_ref_ = command_shaper_.update(track_ref_, {now_secs * 15, 15});
            // track_ref_ = command_shaper_.update(track_ref_, {
            //     1_iq16 * iq16(sin(now_secs / 100)),
            //     0.01_iq16 *  iq16(cos(now_secs / 100))});
                // 1_iq16 * iq16(sin(now_secs)),
                // 1_iq16 *  iq16(cos(now_secs))});
                // _iq16 * iq16(sin(now_secs)),
                // 1_iq16 *  iq16(cos(now_secs))});
            return std::make_tuple(
                iq16::from_bits(track_ref_.x1.to_bits() >> 16),
                track_ref_.x2
            );

            // return std::make_tuple<iq16, iq16>(
            //     amplitude * int(omega * now_secs),
            //     0
            // );
        }();


        [[maybe_unused]] const iq20 torque_cmd = [&]{ 
            const auto kp = MotorProfile::MACHINE_KP;
            const auto kd = MotorProfile::MACHINE_KD;

            // const iq16 e1 = position_cmd - pos_filter_.accumulated_angle().to_turns();
            // const iq16 e2 = speed_cmd - pos_filter_.speed();
            const iq16 e1 = CLAMP2(position_cmd - math::fixed_downcast<16>(rotor_rotation_state_var_.x1), 100);
            const iq16 e2 = CLAMP2(speed_cmd - rotor_rotation_state_var_.x2, 1000);

            return CLAMP2((kp * e1) + (kd * e2), 1);
        }();

        //#endregion 

        //#region 力矩转电流

        [[maybe_unused]] static constexpr iq20 TORQUE_2_current_RATIO = 1_iq20;
        [[maybe_unused]] static constexpr iq20 CURRENT_LIMIT = 1.2_iq16;

        // const iq20 current_cmd = CLAMP2((torque_cmd - math::fixed_downcast<16>(leso_state_var_.x2) / 1000) * TORQUE_2_current_RATIO, CURRENT_LIMIT);
        // const iq20 current_cmd = CLAMP2((torque_cmd) * TORQUE_2_current_RATIO, CURRENT_LIMIT);
        
        const iq20 current_cmd = 0.54_iq20;
        // const iq20 current_cmd = 0.7_iq20;
        // const iq20 current_cmd = 0;
        // const iq20 current_cmd = 0.1_iq20 * iq16(math::sin(now_secs));
        // const iq20 current_cmd = CLAMP2((torque_cmd) * TORQUE_2_current_RATIO, CURRENT_LIMIT);
        // const iq20 current_cmd = 0.07_iq20 * iq16(sin(now_secs));
        // const iq20 current_cmd = 0.07_iq20 * -1;
        //#endregion

        const auto dq_curr = alphabeta_curr.to_dq(elec_rotation);

        [[maybe_unused]] auto generate_dq_volt_by_pi_ctrl = [&]{
            const iq20 dest_d_curr = 0;
            const iq20 dest_q_curr = current_cmd;
            // return DqCoord<iq20>{
            //     .d = d_pi_ctrl_(dest_d_curr - dq_curr.d),
            //     .q = q_pi_ctrl_(dest_q_curr - dq_curr.q)
            // };
            return DqCoord<iq20>{
                // .d = d_pi_ctrl_(dest_d_curr - dq_curr.d) - MotorProfile::PHASE_INDUCTANCE * dq_curr.q * elec_omega,
                // .q = q_pi_ctrl_(dest_q_curr - dq_curr.q) + MotorProfile::PHASE_INDUCTANCE * dq_curr.d * elec_omega
                .d = d_pi_ctrl_(dest_d_curr - dq_curr.d),
                .q = q_pi_ctrl_(dest_q_curr - dq_curr.q)
                // .d = 0,
                // .q = 0
            };
        };

        #if 1
        [[maybe_unused]] auto generate_alpha_beta_volt_by_hfi = [&]{
            // static constexpr uint32_t MASK = (HFI_N) - 1;


            const auto [s,c] = SINCOS32_1_TABLE[hfi_idx];
            const auto sample_now = dot2v2(
                alphabeta_curr.alpha, c,
                alphabeta_curr.beta, s
            );
            if(is_samp_n){
                const auto di = sample_now - prev_sample;
                hfi_buffer[hfi_idx] = di;
                if(hfi_idx >= HFI_N){
                    hfi_idx = 0;
                    const auto buffer_view = std::span(hfi_buffer);
                    const auto hfi_response_real_bin0 = dft32_bin0<20>(buffer_view);
                    // std::tie(hfi_response_real_bin1_, hfi_response_imag_bin1_) = dft32_bin1<20>(buffer_view);
                    // hfi_response_real_bin0_ = lpf_1000hz(hfi_response_real_bin0_, hfi_response_real_bin0);
                    hfi_response_real_bin0_ = hfi_response_real_bin0;
                    
                    const auto [hfi_response_real_bin1, hfi_response_imag_bin1] = dft32_bin1<20>(buffer_view);
                    hfi_response_real_bin1_ = lpf_1000hz(hfi_response_real_bin1_, hfi_response_real_bin1);
                    hfi_response_imag_bin1_ = lpf_1000hz(hfi_response_imag_bin1_, hfi_response_imag_bin1);
                    
                    const auto [hfi_response_real_bin2, hfi_response_imag_bin2] = dft32_bin2<20>(buffer_view);
                    hfi_response_real_bin2_ = lpf_1000hz(hfi_response_real_bin2_, hfi_response_real_bin2);
                    hfi_response_imag_bin2_ = lpf_1000hz(hfi_response_imag_bin2_, hfi_response_imag_bin2);
                }else{
                    hfi_idx += 1;
                }
            }else{
                prev_sample = sample_now;
            }

            if(is_samp_n){
                is_samp_n = false;
                return AlphaBetaCoord<iq20>{
                    .alpha = HFI_VOLT * iq15::from_bits(c.to_bits()),
                    .beta = HFI_VOLT * iq15::from_bits(s.to_bits()),
                };
            }else{
                is_samp_n = true;
                return AlphaBetaCoord<iq20>{
                    .alpha = (-HFI_VOLT) * iq15::from_bits(c.to_bits()),
                    .beta = (-HFI_VOLT) * iq15::from_bits(s.to_bits()),
                };
            }
        };
        #endif


        // const auto dq_volt = (generate_dq_volt_by_hfi()).clamp(MODU_VOLT_LIMIT);
        // const auto dq_volt = (generate_dq_volt_by_pi_ctrl()).clamp(MotorProfile::MODU_VOLT_LIMIT);
        // const auto dq_volt = (generate_dq_volt_by_pi_ctrl()).clamp(MotorProfile::MODU_VOLT_LIMIT);
        const auto dq_volt = DqCoord<iq20>{1.0_iq20, 0};
        // const auto dq_volt = generate_dq_volt_by_constant_voltage().clamp(MotorProfile::MODU_VOLT_LIMIT);

        [[maybe_unused]] const auto hfi_alphabeta_volt = generate_alpha_beta_volt_by_hfi();
        [[maybe_unused]] const auto openloop_alphabeta_volt = [&]{
            // const auto [s,c] = sincospu(-1 * now_secs);
            const auto [s,c] = openloop_elec_angle.sincos();
            return AlphaBetaCoord<iq20>{
                // .alpha = 0.3_iq20 * iq16(c),
                // .beta = 0.3_iq20 * iq16(s),
                .alpha = 1.3_iq20 * iq16(c),
                .beta = 1.3_iq20 * iq16(s),
                // .alpha = 0.0_iq20 * iq16(c),
                // .beta = 0.0_iq20 * iq16(s),
            };
            // return AlphaBetaCoord<iq20>{
            //     .alpha = 0,
            //     .beta = 0,
            // };
        }();

        // auto alphabeta_volt = dq_volt.to_alphabeta(elec_rotation);
        // alphabeta_volt = alphabeta_volt + hfi_alphabeta_volt;

        auto alphabeta_volt = openloop_alphabeta_volt;
        alphabeta_volt = alphabeta_volt + hfi_alphabeta_volt;


        // const auto alphabeta_volt = dq_volt.to_alphabeta(elec_rotation) + generate_alpha_beta_volt_by_hfi();
        // const auto alphabeta_volt = openloop_alphabeta_volt;
        // const auto alphabeta_volt = AlphaBetaCoord<iq20>{1, 0};
        // const auto alphabeta_volt = hfi_alphabeta_volt.clamp(MotorProfile::MODU_VOLT_LIMIT);
        // const auto alphabeta_volt = dq_volt.to_alphabeta(elec_rotation) + generate_alpha_beta_volt_by_hfi();
        // const auto alphabeta_volt = AlphaBetaCoord<iq20>::ZERO;
        // const auto alphabeta_volt = generate_alpha_beta_volt_by_hfi().clamp(MODU_VOLT_LIMIT);

        // flux_sensorless_ob.update(alphabeta_volt, alphabeta_curr);
        // smo_sensorless_ob.update({alphabeta_curr, alphabeta_volt});
        // lbg_sensorless_ob.update({alphabeta_curr, alphabeta_volt});

        const auto uvw_dutycycle = SVM(
            AlphaBetaCoord<iq16>{
                .alpha = alphabeta_volt.alpha, 
                .beta = alphabeta_volt.beta
            } * INV_BUS_VOLT * iq16(1.5)
        );

        
        set_uvw_dutycycle(uvw_dutycycle);

        // leso_state_var_ = leso.iterate(leso_state_var_, rotor_rotation_state_var_.x1, current_cmd);
        leso_state_var_ = leso.iterate(leso_state_var_, rotor_rotation_state_var_.x2, current_cmd);
        // leso_state_var_ = leso.iterate(leso_state_var_, math::fixed_downcast<16>(rotor_rotation_state_var_.x1), current_cmd);
        busbar_current_unfilted_ = UvwCoord<iq20>(iq20(uvw_dutycycle.u), iq20(uvw_dutycycle.v), iq20(uvw_dutycycle.w)).dot(uvw_curr);
        hfi_alphabeta_volt_ = hfi_alphabeta_volt;
        busbar_current_ = lpf_10hz(busbar_current_, busbar_current_unfilted_);
        uvw_dutycycle_ = uvw_dutycycle;
        uvw_current_ = uvw_curr;
        alphabeta_current_ = alphabeta_curr;
        dq_current_ = dq_curr;
        dq_volt_ = dq_volt;
        alphabeta_volt_ = alphabeta_volt;
        openloop_elec_angle_ = openloop_elec_angle;
        encoder_elec_angle_ = encoder_elec_angle;
    };
    static size_t trig_prog = 0;
    auto jeoc_isr = [&]{ 
        timming_watch_pin_.set_high();
        const auto now_us = clock::micros();
        exe_duration_ = now_us - last_exe_us_;
        last_exe_us_ = now_us;

        #if 0
        if(timer.is_up_counting()){
            switch(trig_prog){
                case 0:{
                    timer.oc<4>().set_cvr(full_arr * 1 / 4);

                    trig_prog = 1;
                    break;
                }
                case 1:{
                    timer.oc<4>().set_cvr(full_arr * 2 / 4);

                    trig_prog = 2;
                    break;
                }
                case 2:{
                    timer.oc<4>().set_cvr(full_arr * 3 / 4);
                    trig_prog = 3;
                    break;
                }
                case 3:{
                    timer.oc<4>().set_cvr(10);
                    trig_prog = 0;
                    break;
                }
            }
        }
        #endif
        
        ctrl_isr();
        exe_elapsed_ = clock::micros() - now_us;
        // for(volatile size_t i = 0; i < 30; i++);
        // for(volatile size_t i = 0; i < 6; i++);
        timming_watch_pin_.set_low();
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

    // DBG_UART.set_event_handler([&](const hal::UartEvent & event){
    //     switch(event.kind()){
    //         case hal::UartEvent::TxIdle:{
    //             // __builtin_trap();
    //             // poll_blink_service();
    //             // toggle_blue_led();
    //             break;
    //         }
    //         case hal::UartEvent::RxIdle:{
    //             // __builtin_trap();
    //             // poll_blink_service();
    //             // toggle_blue_led();
    //             break;
    //         }
    //         default:{
    //             break;
    //         }
    //     }
    // });

    set_uvw_dutycycle(UvwCoord<iq16>::HALF);
    timer.start();

    while(true){
        // repl_service_poller();
        // const auto hfi_response_real_bin1 = hfi_response_real_bin1_ * 100;
        // const auto hfi_response_imag_bin1 = hfi_response_imag_bin1_ * 100;
        // const iq20 hfi_response_real_bin1 = 1;
        // const iq20 hfi_response_imag_bin1 = 1;
        // const auto real_bin1_double_ = math::square(hfi_response_real_bin1) - math::square(hfi_response_imag_bin1);
        // const auto imag_bin1_double_ = 2 * ((hfi_response_imag_bin1) * (hfi_response_real_bin1));
        const auto hfi_bin2_angle = Angular<iq31>::from_turns(pu_to_uq32(math::atan2pu(hfi_response_imag_bin2_ - 0.002_iq20, hfi_response_real_bin2_)));
        [[maybe_unused]] const auto hfi_bin2_half_angle = -hfi_bin2_angle / 2;
        // [[maybe_unused]] const auto [sine_hfi_bin2_half_angle, cosine_hfi_bin2_half_angle] = hfi_bin2_half_angle.sincos();
        [[maybe_unused]] const auto length_hfi_response = math::mag(hfi_response_real_bin2_, hfi_response_imag_bin2_) * 2;
        // const auto power_u = ((uvw_dutycycle_.u - 0.5_iq16)* uvw_current_.u);
        // const auto power_v = ((uvw_dutycycle_.v - 0.5_iq16)* uvw_current_.v);
        // const auto power_w = ((uvw_dutycycle_.w - 0.5_iq16)* uvw_current_.w);
        if(true)DEBUG_PRINTLN(
            trig_prog,
            // SINCOS32_2_TABLE[i][0].to_bits(),
            // SINCOS32_2_TABLE[i][1].to_bits(),
            // openloop_elec_angle_.unsigned_normalized().to_turns(),
            // hfi_lap_angle_.to_turns(),
            // hfi_elec_angle_.to_turns(),
            // iq16::from_bits(hfi_response_real_bin0_.to_bits()),
            // hfi_response_real_bin0_.to_bits(),
            // hfi_response_real_bin0_,
            // alphabeta_volt_,
            // uvw_current_bits_offset_,
            // dq_volt_,
            // hfi_alphabeta_volt_,
            // alphabeta_current_,
            // busbar_current_,
            // power_u, power_v, power_w,
            // power_u + power_v + power_w,
            // dq_volt_.d * dq_current_.d,
            // dq_volt_.q * dq_current_.q,
            // (dq_volt_.d * dq_current_.q + dq_volt_.q * dq_current_.d) / 8,
            // hfi_response_real_bin1_,
            // hfi_response_imag_bin1_,
            // hfi_response_real_bin2_,
            // hfi_response_imag_bin2_,
            // alphabeta_current_,
            // (1 / iq16(hfi_response_real_bin0_ + length_hfi_response)) * (1000000 / FOC_FREQ),
            // (1 / iq16(hfi_response_real_bin0_ - length_hfi_response)) * (1000000 / FOC_FREQ),
            // hfi_response_real_bin1,
            // hfi_response_imag_bin1,
            // busbar_current_,
            // uvw_dutycycle_,
            
            // math::atan2pu(hfi_response_imag_bin1 - 0.002_iq20, hfi_response_real_bin1) + 0.5_iq20,
            // hfi_bin2_angle.to_turns(),
            // hfi_bin2_half_angle.to_turns(),
            // cosine_hfi_bin2_half_angle * hfi_response_real_bin1 + sine_hfi_bin2_half_angle * hfi_response_imag_bin1,
            // cosine_hfi_bin2_half_angle * hfi_response_real_bin1 - sine_hfi_bin2_half_angle * hfi_response_imag_bin1,
            // hfi_response_real_bin1 - hfi_response_imag_bin1,
            // hfi_response_real_bin1 + hfi_response_imag_bin1,
            // hfi_response_real_bin1,
            // openloop_elec_angle_.unsigned_normalized().to_turns(),
            // pwm_u_.cvr(),
            // tim_cnt_,
            // timer.arr(),
            // pwm_v_.cvr(),
            // pwm_w_.cvr(),
            // timer.oc<4>().cvr(),
            // smo_sensorless_ob.angle().to_turns(),
            pwm_u_.cvr(),
            pwm_v_.cvr(),
            pwm_w_.cvr(),
            full_arr,
            // exe_duration_.count(),
            exe_elapsed_.count(),
            exe_duration_.count()
            // alphabeta_volt_,
            // alphabeta_current_
            // uvw_current_.u,
            // uvw_current_.v,
            // uvw_current_.w
        );
        // clock::delay(4ms);

        static auto report_timer = async::RepeatTimer::from_duration(5ms);
        if(false)report_timer.invoke_if([&]{
            if(false)DEBUG_PRINTLN(
                pwm_u_.cvr(),
                pwm_v_.cvr(),
                pwm_w_.cvr()
            );
            if(false){
            // if(false){
                DEBUG_PRINTLN(
                    busbar_current_unfilted_,
                    busbar_current_,
                    // encoder_elec_angle_.to_turns()

                    uvw_current_

                );
            }
            if(false){
                const auto now_secs = clock::seconds();
                const auto t = frac(now_secs * 2);
                const auto [s,c] = math::sincospu(t);

                DEBUG_PRINTLN(
                    // math::sin(now_secs),
                    // hal::usart2.available()
                    // SVM_unified(AlphaBetaCoord<iq16>{iq16(c),iq16(s)}),
                    // SVM_reference(AlphaBetaCoord<iq16>{iq16(s),iq16(c)}),
                    SVM(AlphaBetaCoord<iq16>{iq16(c) * iq16(0.866),iq16(s) * iq16(0.866)}),
                    t
                    // u1.count(),
                    // u2.count(),
                    // i
                    // 0
                );
                // DEBUG_PRINTLN(1);
                // const char str[] = "h\r\n";
                // hal::usart2.write_chars(str, sizeof(str) -1);
                // clock::delay(200ms);
            }
            if(true) DEBUG_PRINTLN(
            // if(true) DEBUG_PRINTLN(
                alphabeta_current_,
                // alphabeta_volt_,
                // smo_sensorless_ob.angle().to_turns(),
                uvw_dutycycle_,
                // alphabeta_volt_.beta / alphabeta_current_.beta,
                // dq_current_,
                // dq_volt_,
                // alphabeta_volt_.beta / alphabeta_current_.beta,
                // q_pi_ctrl_.err_sum_,
                // q_pi_ctrl_.kp_
                // q_pi_ctrl_.err_sum_max_
                // lbg_sensorless_ob.angle().to_turns(),
                // hal::adc1.inj<1>().get_voltage(),
                
                // iq16(lap_angle.to_turns()) * POLE_PAIRS,
                // encoder_elec_angle_.to_turns(),
                openloop_elec_angle_.to_turns(),
                iq16::from_bits(track_ref_.x1.to_bits() >> 16),
                // track_ref_.x2,
                iq16::from_bits(rotor_rotation_state_var_.x1.to_bits() >> 16),
                rotor_rotation_state_var_.x2,
                // rotor_rotation_state_var_.x2,
                // leso_state_var_.x2,
                // encoder_lap_angle_.to_turns(),
                // encoder_elec_angle_.to_turns(),
                // encoder_multilap_angle_.to_turns(),
                busbar_current_unfilted_,
                // busbar_current_,
                hfi_response_real_bin2_,
                0
                // encoder_multilap_angle_.to_turns(),
                // leso_state_var_.x2,
                // encoder_elec_angle_.to_turns(),
                // encoder_multilap_angle_.to_turns(),
                // 0
                // flux_sensorless_ob.angle().to_turns()
                // flux_sensorless_ob.V_alphabeta_last_
                // smo_sensorless_ob.angle().to_turns(),
                // exe_elapsed_.count(),

                // flux_sensorless_ob.angle().to_turns(),
                // lbg_sensorless_ob.angle().to_turns(),
                // smo_sensorless_ob.angle().to_turns(),
                // uint32_t(exe_elapsed_.count())

                // openloop_elec_angle_.normalized().to_turns()
                // bool(drv8323_nfault_pin_.read() == LOW),


                // pwm_u_.cvr(),
                // pwm_v_.cvr(),
                // pwm_w_.cvr(),

                // pwm_u_.get_dutycycle(),
                // pwm_v_.get_dutycycle(),
                // pwm_w_.get_dutycycle(),
            );
        });

        poll_blink_service();
        // toggle_red_led();
        // repl_service_poller();
        // clock::delay(2ms);
    }

}