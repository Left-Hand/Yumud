#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/utils/default.hpp"
#include "core/async/timer.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/conn/spi/hw_singleton.hpp"
#include "hal/dma/dma.hpp"

#include "dsp/fft/fft32.hpp"
#include "dsp/motor_ctrl/sensorless/slide_mode_observer.hpp"
#include "dsp/motor_ctrl/sensorless/luenberger_observer.hpp"
#include "dsp/motor_ctrl/sensorless/nonlinear_flux_observer.hpp"

#include "dsp/controller/adrc/nonlinear/nltd2o.hpp"
#include "dsp/controller/adrc/linear/ltd2o.hpp"
#include "dsp/filter/firstorder/lpf.hpp"
#include "dsp/filter/butterworth/band.hpp"

#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"
#include "digipw/ctrl/pi_controller.hpp"

#include "linear_regression.hpp"
#include "motor_profiles.hpp"

#include "core/sdk.hpp"


#include "motor_dsp/dsp_lpf.hpp"
#include "motor_dsp/dsp_vec.hpp"


#include "drivers/gatedrv/DRV832X/DRV8323h.hpp"


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
using MotorProfile = MotorProfile_Wheel;
// using MotorProfile = MotorProfile_3505;
// using MotorProfile = MotorProfile_36BLDB;
// using MotorProfile = MotorProfile_NiuLiu;


struct LrSeriesCurrentRegulatorConfig{
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 截止频率/带宽 (Hz)
    iq20 phase_inductance;        // 相电感 (H)
    iq20 phase_resistance;        // 相电阻 (Ω)
    iq16 voltage_limit;                // 最大电压 (V)

    [[nodiscard]] constexpr Result<digipw::PiController::Cofficients, StringView> 
    try_into_precomputed() const noexcept {
        //U(s) = I(s) * R + s * I(s) * L
        //I(s) / U(s) = 1 / (R + sL)
        //G_open(s) = (Ki / s + Kp) / s(R / s + L)

        // Ki = 2pi * fc * R
        // Kp = 2pi * fc * L

        if(fs >= 65535) return Err(StringView("fs too large"));
        if(fc * 8 >= fs) return Err(StringView("fc too large"));

        const auto & self = *this;
        digipw::PiController::Cofficients coeffs;

        //norm_omega = fc * 2pi / fs

        const auto norm_omega = uq24::from_bits(static_cast<uint32_t>((static_cast<uint64_t>(uq24(TAU).to_bits()) * fc) / self.fs));


        coeffs.max_out = self.voltage_limit;

        coeffs.kp = iq20(self.phase_inductance * self.fc) * iq16(TAU);
        coeffs.ki_discrete = self.phase_resistance * norm_omega;

        coeffs.err_int_max = coeffs.max_out / iq16(coeffs.ki_discrete);
        return Ok(coeffs);
    }
};



template<size_t FC, size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_specified_fc(
    const math::fixed<Q, int32_t> x_state,
    const math::fixed<Q, int32_t> x_new
){
    constexpr auto ALPHA = dsp::calc_lpf_alpha_uq32(FOC_FREQ, FC).unwrap();
    return lpf_1o(x_state, x_new, ALPHA);
}


template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_10hz(
    math::fixed<Q, int32_t> x_state,
    const math::fixed<Q, int32_t> x_new
){
    return lpf_specified_fc<10>(x_state, x_new);
}

template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_100hz(
    math::fixed<Q, int32_t> x_state, const math::fixed<Q, int32_t> x_new
){
    return lpf_specified_fc<100>(x_state, x_new);
}

template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_1000hz(
    math::fixed<Q, int32_t> x_state, const math::fixed<Q, int32_t> x_new
){
    return lpf_specified_fc<1000>(x_state, x_new);
}

template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_allpass(
    math::fixed<Q, int32_t> x_state, const math::fixed<Q, int32_t> x_new
){
    return x_new;
}


static constexpr auto CURRENT_REGULATOR_CFG = LrSeriesCurrentRegulatorConfig{
    .fs = FOC_FREQ,
    .fc = MotorProfile::CURRENT_CUTOFF_FREQ,
    .phase_inductance = MotorProfile::PHASE_INDUCTANCE,
    .phase_resistance = MotorProfile::PHASE_RESISTANCE,
    .voltage_limit = MotorProfile::MODU_VOLT_LIMIT,
};

static constexpr auto PI_CONTROLLER_COEFFS = CURRENT_REGULATOR_CFG.try_into_precomputed().unwrap();

static constexpr size_t HFI_N = 32;
static_assert(std::has_single_bit(HFI_N));

static void init_adc(){

    hal::adc1.init({
            {hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T28_5}
        },{

            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T13_5},
            {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T13_5},
            {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T13_5},  
            {hal::AdcChannelSelection::TEMP, hal::AdcSampleCycles::T13_5},  

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
        // .baudrate = hal::NearestFreq(6000000),
        .baudrate = hal::NearestFreq(576000),
        .rx_strategy = CommStrategy::Dma,
        .tx_strategy = CommStrategy::Blocking,
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.build_config()
        .set_eps(4)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();
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
    timer.oc<4>().cvr() = timer.arr() - 2;
    // timer.oc<4>().cvr() = 2;
    // timer.oc<4>().cvr() = timer.arr() / 2;

    timer.oc<4>().enable_output(EN);

    auto set_uvw_dutycycle = [&](const UvwCoord<uq16> & dutycycle){
        // timer.enable_udis(DISEN);
        auto * inst = TIM1;
        const uint16_t arr = static_cast<uint16_t>(inst->ATRLR);
        
        auto convert = [&](const uq16 channel_dutycycle) -> uint16_t{
            return static_cast<uint16_t>(channel_dutycycle * arr);
        };

        inst->CH1CVR = convert(dutycycle.template get<0>());
        inst->CH2CVR = convert(dutycycle.template get<1>());
        inst->CH3CVR = convert(dutycycle.template get<2>());
    };

    auto stop_pwm = [&]{
        timer.stop();
    };

    // #endregion 初始化定时器

    // #region 初始化ADC


    init_adc();

    auto & u_current_sense_ch_ = hal::adc1.inj<1>();
    auto & v_current_sense_ch_ = hal::adc1.inj<2>();
    auto & w_current_sense_ch_ = hal::adc1.inj<3>();

    auto get_uvw_current_u12x3 = [&] -> std::array<uint32_t, 3>{
        return {
            u_current_sense_ch_.read_u12(),
            v_current_sense_ch_.read_u12(),
            w_current_sense_ch_.read_u12()
        };
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
    drv8323_gain_pin_.inflt();//20x
    // drv8323_gain_pin_.outpp(HIGH);//40x


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

    [[maybe_unused]] static constexpr iq20 HFI_VOLT_LIMIT = MotorProfile::PHASE_RESISTANCE * 1.4_iq20;
    // static constexpr iq20 HFI_VOLT = MIN((, 2);
    static constexpr iq20 HFI_VOLT = 0.0_iq20;
    // static constexpr iq20 HFI_VOLT = 0;
    static iq20 prev_sample = Zero;
    static size_t hfi_idx = 0;
    static std::array<iq20, HFI_N> hfi_buffer;
    static bool is_samp_n = false;



    // PANIC{PI_CONTROLLER_COEFFS};

    auto d_pi_ctrl_ = PI_CONTROLLER_COEFFS.to_pi_controller();
    auto q_pi_ctrl_ = PI_CONTROLLER_COEFFS.to_pi_controller();


    #if 0
    constexpr auto FLUX_SENSORLESS_OB_COEFFS = dsp::motor_ctl::NonlinearFluxObserver::Config{
        .fs = FOC_FREQ,
        .phase_inductance = MotorProfile::PHASE_INDUCTANCE,
        .phase_resistance = MotorProfile::PHASE_RESISTANCE,

        // .observer_gain = 0.16_iq20, // [rad/s]
        // .pm_flux_linkage = 0.000017_iq20, // [V / (rad/s)]

        #if 0
        .observer_gain = 0.1201_iq20, // [rad/s]
        .pm_flux_linkage = 0.00084_iq20, // [V / (rad/s)]
        #else
        // .observer_gain = 0.0101_iq20, // [rad/s]
        .observer_gain = 0.1023_iq20, // [rad/s]
        .pm_flux_linkage = 0.00877_iq20, // [V / (rad/s)]
        #endif
    }.try_into_precomputed().unwrap();
    #else
    [[maybe_unused]] constexpr auto FLUX_SENSORLESS_OB_COEFFS = dsp::motor_ctl::NonlinearFluxObserver::ConfigF32{
        .fs = FOC_FREQ,
        .phase_inductance = float(MotorProfile::PHASE_INDUCTANCE),
        .phase_resistance = float(MotorProfile::PHASE_RESISTANCE),

        // .observer_gain = 10.1023, // [rad/s]
        .observer_gain = 0.1023, // [rad/s]
        .pm_flux_linkage = 0.00877, // [V / (rad/s)]
        // .pm_flux_linkage = 0.01277, // [V / (rad/s)]
    }.try_into_precomputed().unwrap();
    #endif

    #if 0
    [[maybe_unused]] auto flux_sensorless_ob = dsp::motor_ctl::NonlinearFluxObserver{
        FLUX_SENSORLESS_OB_COEFFS
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
    #endif

    using Nltd2o = NonlinearTrackingDifferentiator<iq16, 2>;

    #if 0

    Leso leso = Leso{
        MotorProfile::LESO_COEFFS
    };

    Leso::State leso_state_var_ = Zero;


    static constexpr auto command_shaper_coeffs = Nltd2o::Config{
        .fs = FOC_FREQ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        // .r = 152.5_iq10,
        .r = 202.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 240
    }.try_into_precomputed().unwrap();
    #else
    static constexpr auto command_shaper_coeffs = Nltd2o::Config{
        .fs = FOC_FREQ,
        .r = 587.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 40
    }.try_into_precomputed().unwrap();

    #endif

    static constexpr Nltd2o command_shaper_{
        command_shaper_coeffs
    };


    using Ltd2o = dsp::adrc::LinearTrackingDifferentiator<iq16, 2>;
    static constexpr auto rotor_rotation_ltd_coeffs = Ltd2o::Config{
        .fs = FOC_FREQ, .r = 500
    }.try_into_precomputed().unwrap();

    [[maybe_unused]] static constexpr Ltd2o rotor_rotation_ltd_{
        rotor_rotation_ltd_coeffs
    };

    static constexpr size_t DC_CAL_TIMES = 32 * 128;

    struct MotorState{
        SecondOrderState<iq16> track_ref;
        SecondOrderState<iq16> rotor_rotation_state_var;

        Angular<uq32> encoder_elec_angle;
        Angular<uq32> hfi_elec_angle;
        Angular<uq32> openloop_elec_angle;
        Angular<uq32> selected_elec_angle;

        Angular<uq32> encoder_lap_angle;
        Angular<iiq32> encoder_multilap_angle;

        Angular<uq16> hfi_lap_angle;
        Angular<iq16> hfi_multilap_angle;

        UvwCoord<iq20> uvw_curr_unfilted;
        DqCoord<iq20> dq_curr_unfilted;
        DqCoord<iq20> dq_volt_gen;

        AlphaBetaCoord<iq20> alphabeta_curr_unfilted;
        AlphaBetaCoord<iq20> hfi_alphabeta_volt;
        AlphaBetaCoord<iq20> alphabeta_volt_gen;
        UvwCoord<uq16> uvw_dutycycle_gen;


        iq20 busbar_curr_unfilted;
        iq20 busbar_curr;
        iq20 torque_curr_cmd;



        iq20 hfi_response_real_bin0;
        iq20 hfi_response_real_bin1;
        iq20 hfi_response_imag_bin1;
        iq20 hfi_response_real_bin2;
        iq20 hfi_response_imag_bin2;

        Microseconds exe_elapsed_us;
        Microseconds last_exe_begin_us;
        Microseconds exe_duration;
        std::array<uint32_t, 3> uvw_current_bits_offset_acc;
        std::array<uint16_t, 3> uvw_current_bits_offset;
    
        bool dc_cal_done;
        size_t dc_cal_cnt;

        void set_default(){
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wclass-memaccess"
            memset(this, 0, sizeof(*this));
            #pragma GCC diagnostic pop
        }
    };

    MotorState motor_state;
    motor_state.set_default();


    #if 0
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
            // track_ref = command_shaper_.update(track_ref, );
            track_ref = command_shaper_.iterate(track_ref, {
                x1_cmd,
                x2_cmd
            });
            // track_ref = command_shaper_.update(track_ref, {now_secs * 15, 15});
            // track_ref = command_shaper_.update(track_ref, {
            //     1_iq16 * iq16(sin(now_secs / 100)),
            //     0.01_iq16 *  iq16(cos(now_secs / 100))});
                // 1_iq16 * iq16(sin(now_secs)),
                // 1_iq16 *  iq16(cos(now_secs))});
                // _iq16 * iq16(sin(now_secs)),
                // 1_iq16 *  iq16(cos(now_secs))});
            return std::make_tuple(
                iq16::from_bits(track_ref.x1.to_bits() >> 16),
                track_ref.x2
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
            const iq16 e1 = CLAMP2(position_cmd - math::fixed_downcast<16>(state.rotor_rotation_state_var.x1), 100);
            const iq16 e2 = CLAMP2(speed_cmd - state.rotor_rotation_state_var.x2, 1000);

            return CLAMP2((kp * e1) + (kd * e2), 1);
        }();
    #endif


    [[maybe_unused]] auto current_sense = [&](MotorState & state){
        const auto uvw_current_u12x3 = get_uvw_current_u12x3();
        if(state.dc_cal_done == false) [[unlikely]]{
            state.uvw_current_bits_offset_acc = {
                std::get<0>(state.uvw_current_bits_offset_acc) + static_cast<uint32_t>(std::get<0>(uvw_current_u12x3)),
                std::get<1>(state.uvw_current_bits_offset_acc) + static_cast<uint32_t>(std::get<1>(uvw_current_u12x3)),
                std::get<2>(state.uvw_current_bits_offset_acc) + static_cast<uint32_t>(std::get<2>(uvw_current_u12x3))
            };
            state.dc_cal_cnt++;
            if(state.dc_cal_cnt >= DC_CAL_TIMES){
                state.uvw_current_bits_offset = {
                    static_cast<uint16_t>(std::get<0>(state.uvw_current_bits_offset_acc) / int32_t(DC_CAL_TIMES)),
                    static_cast<uint16_t>(std::get<1>(state.uvw_current_bits_offset_acc) / int32_t(DC_CAL_TIMES)),
                    static_cast<uint16_t>(std::get<2>(state.uvw_current_bits_offset_acc) / int32_t(DC_CAL_TIMES))
                };
                state.dc_cal_done = true;
            }

            {
                const auto LOW_HFI_VOLT = iq20(0.6_iq16);
                const auto [s,c] = dsp::DFT32_BIN1_SINCOS_TABLE[state.dc_cal_cnt & 0x1f];
                const auto alphabeta_volt_gen = AlphaBetaCoord<iq16>{
                    .alpha = iq15::from_bits(s.to_bits()) * LOW_HFI_VOLT,
                    .beta = iq15::from_bits(c.to_bits()) * LOW_HFI_VOLT
                };

                set_uvw_dutycycle(SVM(alphabeta_volt_gen * INV_BUS_VOLT * iq16(1.5)));
            }
            return;
        }


        //#region 电流传感


        state.uvw_curr_unfilted = UvwCoord<iq20>{
            .u = (int32_t(uvw_current_u12x3[0]) - int32_t((state.uvw_current_bits_offset[0])))
                 * CURRENT_AMPS_PER_ADC_LSB,
            .v = (int32_t(uvw_current_u12x3[1]) - int32_t((state.uvw_current_bits_offset[1])))
                 * CURRENT_AMPS_PER_ADC_LSB,
            .w = (int32_t(uvw_current_u12x3[2]) - int32_t((state.uvw_current_bits_offset[2])))
                 * CURRENT_AMPS_PER_ADC_LSB,
        };

        state.alphabeta_curr_unfilted = AlphaBetaCoord<iq20>::from_uvw(state.uvw_curr_unfilted);
        // const auto alphabeta_curr_unfilted = alphabeta_curr_unfilted;
        // const auto alphabeta_curr_unfilted = AlphaBetaCoord{
        //     lpf_1o(state.alphabeta_curr_unfilted.alpha, alphabeta_curr_unfilted.alpha),
        //     lpf_1o(state.alphabeta_curr_unfilted.beta, alphabeta_curr_unfilted.beta),
        // };
        //#endregion
    };
    [[maybe_unused]] auto position_loop = [&](MotorState & state){
        //#region 力矩转电流

        [[maybe_unused]] static constexpr iq20 TORQUE_2_current_RATIO = 1_iq20;
        [[maybe_unused]] static constexpr iq20 CURRENT_LIMIT = 1.2_iq16;

        // const iq20 state.torque_curr_cmd = CLAMP2((torque_cmd - math::fixed_downcast<16>(leso_state_var_.x2) / 1000) * TORQUE_2_current_RATIO, CURRENT_LIMIT);
        // const iq20 state.torque_curr_cmd = CLAMP2((torque_cmd) * TORQUE_2_current_RATIO, CURRENT_LIMIT);

        // const iq20 state.torque_curr_cmd = 0.24_iq20;
        // const iq20 state.torque_curr_cmd = 0.7_iq20;
        // const iq20 state.torque_curr_cmd = 0;
        // const iq20 state.torque_curr_cmd = 0.1_iq20 * iq16(math::sin(now_secs));
        // const iq20 state.torque_curr_cmd = CLAMP2((torque_cmd) * TORQUE_2_current_RATIO, CURRENT_LIMIT);
        // const iq20 state.torque_curr_cmd = 0.07_iq20 * iq16(sin(now_secs));
        // const iq20 state.torque_curr_cmd = 0.07_iq20 * -1;
        //#endregion
    };


    [[maybe_unused]] auto ob_loop = [&](MotorState & state){
        // leso_state_var_ = leso.iterate(leso_state_var_, state.rotor_rotation_state_var.x1, state.torque_curr_cmd);
        // leso_state_var_ = leso.iterate(leso_state_var_, state.rotor_rotation_state_var.x2, state.torque_curr_cmd);
        // leso_state_var_ = leso.iterate(leso_state_var_, math::fixed_downcast<16>(state.rotor_rotation_state_var.x1), state.torque_curr_cmd);
    };


    [[maybe_unused]] auto current_loop = [&](MotorState & state){


        //#region 位置提取

        [[maybe_unused]] const auto now_secs = clock::seconds();
        [[maybe_unused]] static constexpr auto ZERO_ELEC_ANGLE = Angular<uq32>::ZERO;
        const auto hfi_angle = Angular<uq32>::from_turns(
            math::pu_to_uq32(math::atan2pu(state.hfi_response_imag_bin2 - 0.002_iq20, state.hfi_response_real_bin2))
        );


        if(false){
            if(false){
                const auto next_encoder_lapturns = mag_encoder_.read_lap_angle().examine().to_turns();

                const auto encoder_diff_angle = uq32_wrapped_diff(state.encoder_lap_angle.to_turns(), next_encoder_lapturns);

                state.encoder_lap_angle = Angular<uq32>::from_turns(next_encoder_lapturns);
                state.encoder_multilap_angle = Angular<iiq32>::from_turns(state.encoder_multilap_angle.to_turns() + encoder_diff_angle);
                rotor_rotation_ltd_.iterate(
                    state.rotor_rotation_state_var,
                    {fixed_downcast<16>(state.encoder_multilap_angle.to_turns()), 0}
                );


                state.encoder_elec_angle = Angular<uq32>::from_turns(math::pu_to_uq32(next_encoder_lapturns * MotorProfile::POLE_PAIRS)
                        + MotorProfile::SENSORED_ELEC_ANGLE_BASE.to_turns());
            }else{
                const auto next_hfi_lap_angle = hfi_angle.cast_inner<uq16>();

                const auto hfi_diff_angle = (next_hfi_lap_angle.cast_inner<iq16>()
                    - state.hfi_lap_angle.cast_inner<iq16>()).signed_normalized() / 2;

                state.hfi_lap_angle = next_hfi_lap_angle;
                state.hfi_multilap_angle = state.hfi_multilap_angle + hfi_diff_angle;
                rotor_rotation_ltd_.iterate(state.rotor_rotation_state_var, {state.hfi_multilap_angle.to_turns(), 0});

                state.hfi_elec_angle = make_angular_from_turns(uq32((state.hfi_multilap_angle).unsigned_normalized().to_turns()));
            }
        }else{
            const auto openloop_manchine_multilap_angle = Angular<iq16>::from_turns(0.10_iq16 * now_secs);

            const auto openloop_elec_angle = make_angular_from_turns(
                uq32(frac(openloop_manchine_multilap_angle.to_turns()) * MotorProfile::POLE_PAIRS));


            state.openloop_elec_angle = openloop_elec_angle;
        }

        state.selected_elec_angle = state.openloop_elec_angle;


        const auto elec_sincos = math::Rotation2<iq16>::from_angle(state.selected_elec_angle);

        //#endregion

        const auto dq_curr_unfilted = state.alphabeta_curr_unfilted.to_dq(elec_sincos);

        [[maybe_unused]] auto generate_dq_volt_by_pi_ctrl = [&]{
            const iq20 dest_d_curr = 0;
            const iq20 dest_q_curr = state.torque_curr_cmd;
            // return DqCoord<iq20>{
            //     .d = d_pi_ctrl_(dest_d_curr - dq_curr_unfilted.d),
            //     .q = q_pi_ctrl_(dest_q_curr - dq_curr_unfilted.q)
            // };
            return DqCoord<iq20>{
                // .d = d_pi_ctrl_(dest_d_curr - dq_curr_unfilted.d) - MotorProfile::PHASE_INDUCTANCE * dq_curr_unfilted.q * elec_omega,
                // .q = q_pi_ctrl_(dest_q_curr - dq_curr_unfilted.q) + MotorProfile::PHASE_INDUCTANCE * dq_curr_unfilted.d * elec_omega
                .d = d_pi_ctrl_(dest_d_curr - dq_curr_unfilted.d),
                .q = q_pi_ctrl_(dest_q_curr - dq_curr_unfilted.q)
                // .d = 0,
                // .q = 0
            };
        };

        #if 1
        [[maybe_unused]] auto generate_alpha_beta_volt_by_hfi = [&]{
            // static constexpr uint32_t MASK = (HFI_N) - 1;


            const auto [s,c] = DFT32_BIN1_SINCOS_TABLE[hfi_idx];
            const auto sample_now = dot2v2(
                state.alphabeta_curr_unfilted.alpha, c,
                state.alphabeta_curr_unfilted.beta, s
            );
            if(is_samp_n){
                const auto di = sample_now - prev_sample;
                hfi_buffer[hfi_idx] = di;
                if(hfi_idx >= HFI_N){
                    hfi_idx = 0;
                    const auto buffer_view = std::span(hfi_buffer);
                    const auto hfi_response_real_bin0 = dft32_bin0<20>(buffer_view);
                    // std::tie(state.hfi_response_real_bin1, state.hfi_response_imag_bin1) = dft32_bin1<20>(buffer_view);
                    // state.hfi_response_real_bin0 = lpf_1000hz(state.hfi_response_real_bin0, hfi_response_real_bin0);
                    state.hfi_response_real_bin0 = hfi_response_real_bin0;

                    const auto [hfi_response_real_bin1, hfi_response_imag_bin1] = dft32_bin1<20>(buffer_view);
                    state.hfi_response_real_bin1 = lpf_1000hz(state.hfi_response_real_bin1, hfi_response_real_bin1);
                    state.hfi_response_imag_bin1 = lpf_1000hz(state.hfi_response_imag_bin1, hfi_response_imag_bin1);

                    const auto [hfi_response_real_bin2, hfi_response_imag_bin2] = dft32_bin2<20>(buffer_view);
                    state.hfi_response_real_bin2 = lpf_1000hz(state.hfi_response_real_bin2, hfi_response_real_bin2);
                    state.hfi_response_imag_bin2 = lpf_1000hz(state.hfi_response_imag_bin2, hfi_response_imag_bin2);
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


        // const auto dq_volt_gen = (generate_dq_volt_by_hfi()).clamp(MODU_VOLT_LIMIT);
        // const auto dq_volt_gen = (generate_dq_volt_by_pi_ctrl()).clamp(MotorProfile::MODU_VOLT_LIMIT);
        auto dq_volt_gen = generate_dq_volt_by_pi_ctrl();
        
        const bool cross_decoupling_enabled = true;

        if(cross_decoupling_enabled){
            const bool is_speed_stable = true;
            const auto omega = state.rotor_rotation_state_var.x2 * is_speed_stable;
            dq_volt_gen.d -= MotorProfile::PHASE_INDUCTANCE * dq_curr_unfilted.q * omega;
            dq_volt_gen.q += MotorProfile::PHASE_INDUCTANCE * dq_curr_unfilted.d * omega;
        }

        const bool bemf_decoupling_enabled = true;
        if(bemf_decoupling_enabled){
            const bool is_speed_stable = true;
            const auto omega = state.rotor_rotation_state_var.x2 * is_speed_stable;
           dq_volt_gen.q += MotorProfile::FLUX_LINKAGE * omega;
        }


        dq_volt_gen = dq_volt_gen.clamp(MotorProfile::MODU_VOLT_LIMIT);

        const auto inv_bus_volt_3_by2 = INV_BUS_VOLT * iq16(1.5);

        auto dq_dutycycle_gen = dq_volt_gen * inv_bus_volt_3_by2;

        auto alphabeta_dutycycle_gen = dq_dutycycle_gen.to_alphabeta(elec_sincos);



        const auto uvw_dutycycle_gen = SVM(
            alphabeta_dutycycle_gen
        );


        set_uvw_dutycycle(uvw_dutycycle_gen);


        state.busbar_curr_unfilted = UvwCoord<iq20>(iq20(uvw_dutycycle_gen.u), iq20(uvw_dutycycle_gen.v), iq20(uvw_dutycycle_gen.w)).dot(state.uvw_curr_unfilted);
        state.busbar_curr = lpf_10hz(state.busbar_curr, state.busbar_curr_unfilted);
        state.uvw_dutycycle_gen = uvw_dutycycle_gen;

        state.dq_curr_unfilted = dq_curr_unfilted;
        state.dq_volt_gen = dq_volt_gen;
        // state.alphabeta_volt_gen = alphabeta_volt_gen;

        // flux_sensorless_ob.update(alphabeta_volt_gen, alphabeta_curr_unfilted);
        // smo_sensorless_ob.update({alphabeta_curr_unfilted, alphabeta_volt_gen});
        // lbg_sensorless_ob.update({alphabeta_curr_unfilted, alphabeta_volt_gen});
    };

    [[maybe_unused]] static size_t trig_prog = 0;
    auto jeoc_isr = [&]{
        timming_watch_pin_.set_high();
        const auto exe_begin_us = clock::micros();
        auto  & state = motor_state;
        state.exe_duration = exe_begin_us - state.last_exe_begin_us;
        state.last_exe_begin_us = exe_begin_us;

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

        current_sense(state);
        position_loop(state);
        current_loop(state);
        state.exe_elapsed_us = clock::micros() - exe_begin_us;
        // for(volatile size_t i = 0; i < 30; i++);
        // for(volatile size_t i = 0; i < 6; i++);
        timming_watch_pin_.set_low();
    };

    hal::adc1.register_nvic(hal::NvicPriorityCode::highest(),  EN);
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

    set_uvw_dutycycle(UvwCoord<uq16>::HALF);
    timer.start();

    clock::delay(2ms);

    const auto temp_comp = hal::TemperatureCompensator::load();
    iq16 temperature_ = temp_comp.comp_u12(hal::adc1.inj<4>().read_u12());


    [[maybe_unused]] auto poll_repl_activity = [&]{
        [[maybe_unused]] static repl::ReplServer repl_server{&DBG_UART, &DBG_UART};
        repl_server.enable_echo(DISEN);

        [[maybe_unused]] static const auto list = script::make_list(
            "root",
            script::make_function(StringView("cm"), [&](iq20 torque_curr_cmd){
                // handle_start_advanced_task();
                if(math::abs(torque_curr_cmd) > 10) return;
                motor_state.torque_curr_cmd = torque_curr_cmd;
            })
        );

        repl_server.invoke(list);
    };
    
    while(true){
        poll_repl_activity();
        [[maybe_unused]] const auto now_secs = clock::seconds();
        // repl_service_poller();
        // const auto hfi_response_real_bin1 = motor_state.hfi_response_real_bin1 * 100;
        // const auto hfi_response_imag_bin1 = motor_state.hfi_response_imag_bin1 * 100;
        // const iq20 hfi_response_real_bin1 = 1;
        // const iq20 hfi_response_imag_bin1 = 1;
        // const auto real_bin1_double_ = math::square(hfi_response_real_bin1) - math::square(hfi_response_imag_bin1);
        // const auto imag_bin1_double_ = 2 * ((hfi_response_imag_bin1) * (hfi_response_real_bin1));
        [[maybe_unused]] const auto hfi_bin2_angle = Angular<uq32>::from_turns(
            pu_to_uq32(math::atan2pu(
                motor_state.hfi_response_imag_bin2 - 0.002_iq20, 
                motor_state.hfi_response_real_bin2)
            )
        );
        // [[maybe_unused]] const auto hfi_bin2_half_angle = -hfi_bin2_angle / 2;
        // [[maybe_unused]] const auto [sine_hfi_bin2_half_angle, cosine_hfi_bin2_half_angle] = hfi_bin2_half_angle.sincos();
        [[maybe_unused]] const auto length_hfi_response = math::mag(motor_state.hfi_response_real_bin2, motor_state.hfi_response_imag_bin2) * 2;
        // const auto power_u = ((motor_state.uvw_dutycycle_gen.u - 0.5_iq16)* motor_state.uvw_curr_unfilted.u);
        // const auto power_v = ((motor_state.uvw_dutycycle_gen.v - 0.5_iq16)* motor_state.uvw_curr_unfilted.v);
        // const auto power_w = ((motor_state.uvw_dutycycle_gen.w - 0.5_iq16)* motor_state.uvw_curr_unfilted.w);
        temperature_ = lpf_10hz(temperature_, temp_comp.comp_u12(hal::adc1.inj<4>().read_u12()));
        if(true)DEBUG_PRINTLN(
            // s, c, 
            pwm_u_.cvr(),
            pwm_v_.cvr(),
            pwm_w_.cvr(),
            // motor_state.uvw_curr_unfilted.u,
            // motor_state.uvw_curr_unfilted.v,
            // motor_state.uvw_curr_unfilted.w,
            // motor_state.alphabeta_volt_gen.alpha,
            // motor_state.alphabeta_volt_gen.beta,
            // motor_state.dq_volt_gen.d,
            // motor_state.dq_volt_gen.q,
            // flux_sensorless_ob.angle().to_turns(),
            // flux_sensorless_ob.motor_state().flux_state_mf[0],
            // flux_sensorless_ob.motor_state().flux_state_mf[1],
            // flux_sensorless_ob.motor_state().v_alphabeta_last[0],
            // flux_sensorless_ob.motor_state().v_alphabeta_last[1],
            motor_state.torque_curr_cmd,
            motor_state.dq_curr_unfilted.d,
            motor_state.dq_curr_unfilted.q,
            motor_state.alphabeta_curr_unfilted[0],
            motor_state.alphabeta_curr_unfilted[1],
            // motor_state.busbar_curr,
            // hfi_bin2_angle.to_turns()
            // motor_state.hfi_response_imag_bin2,
            // motor_state.hfi_response_real_bin2
            // full_arr,
            // motor_state.exe_duration.count(),
            
            // temperature_,
            // flux_sensorless_ob.angle().to_turns(),
            // math::atan2pu(motor_state.alphabeta_curr_unfilted[0], motor_state.alphabeta_curr_unfilted[1]),
            // motor_state.alphabeta_curr_unfilted[0] / motor_state.alphabeta_curr_unfilted[1],
            static_cast<uint32_t>(motor_state.exe_elapsed_us.count())
            // motor_state.exe_duration.count()
            // motor_state.alphabeta_volt_gen,
            // motor_state.alphabeta_curr_unfilted
            // motor_state.uvw_curr_unfilted.u,
            // motor_state.uvw_curr_unfilted.v,
            // motor_state.uvw_curr_unfilted.w
        );
        // clock::delay(4ms);

        poll_blink_service();
        // toggle_red_led();
        // repl_service_poller();
        // clock::delay(2ms);
    }

}
