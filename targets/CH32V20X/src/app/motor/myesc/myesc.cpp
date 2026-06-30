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

#include "motor_config.hpp"

#include "core/sdk.hpp"


#include "motor_dsp/dsp_lpf.hpp"
#include "motor_dsp/dsp_vec.hpp"
#include "motor_dsp/dsp_fft32.hpp"



#include "drivers/gatedrv/DRV832X/DRV8323h.hpp"


using namespace ymd;

using namespace ymd::drivers;

using namespace ymd::dsp::adrc;
using namespace ymd::myesc;




#define DBG_UART hal::usart2


using Leso = ymd::dsp::adrc::MotorLeso;


struct LrSeriesCurrentRegulatorConfig{
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 截止频率/带宽 (Hz)
    iq20 phase_inductance_mh;        // 相电感 (mH)
    iq20 phase_resistance_ohm;        // 相电阻 (Ω)

    [[nodiscard]] constexpr Result<digipw::PiCofficients, StringView> 
    try_into_precomputed() const noexcept {
        //U(s) = I(s) * R + s * I(s) * L
        //I(s) / U(s) = 1 / (R + sL)
        //G_open(s) = (Ki / s + Kp) / s(R / s + L)

        // Ki = 2pi * fc * R
        // Kp = 2pi * fc * L

        if(fs >= 65535) return Err(StringView("fs too large"));
        if(fc * 8 >= fs) return Err(StringView("fc too large"));

        const auto & self = *this;
        digipw::PiCofficients coeffs;

        //norm_omega = fc * 2pi / fs

        const auto factor = (uq16::from_bits(TAU_SCALE_NUM * fc)
            / uq16::from_bits(TAU_SCALE_DEN * 1000));

        const auto norm_omega = uq32::from_bits(TAU_SCALE_NUM * fc) 
            / uq32::from_bits(TAU_SCALE_DEN * self.fs);

        coeffs.kp = self.phase_inductance_mh * factor;
        coeffs.ki_discrete = self.phase_resistance_ohm * norm_omega;                                    
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
static constexpr math::fixed<Q, int32_t> lpf_50hz(
    math::fixed<Q, int32_t> x_state, const math::fixed<Q, int32_t> x_new
){
    return lpf_specified_fc<50>(x_state, x_new);
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
    .phase_inductance_mh = MotorProfile::PHASE_INDUCTANCE_MH,
    .phase_resistance_ohm = MotorProfile::PHASE_RESISTANCE_OHM,
};

static constexpr auto PI_CONTROLLER_COEFFS = CURRENT_REGULATOR_CFG.try_into_precomputed().unwrap();

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




static constexpr bool judge_is_disconn(const iq20 meas, const iq20 ref){
    const auto abs_ref = math::abs(ref);
    if(abs_ref < CURRENT_NOISE_STDVAR) return false;
    const auto abs_meas = math::abs(meas);
    return (abs_meas * 30 < abs_ref) and (abs_meas < CURRENT_NOISE_STDVAR);
}




iq16 _tmrticks_to_us(const int32_t counter_value){
    static constexpr uint32_t factor = (1ull << 32) * (1.0 / 144);
    return iq16::from_bits(int32_t((int64_t(counter_value) * factor) >> 16));
}

iq16 tmrticks_to_us(const TimerTick tick){
    int32_t counter_value = int32_t(tick.counter_value);
    if(tick.is_up_counting) counter_value = TIMER_ARR_VALUE + counter_value;
    else counter_value = TIMER_ARR_VALUE - counter_value;
    return _tmrticks_to_us(counter_value);
}



static constexpr size_t HFI_N = 32;
static_assert(std::has_single_bit(HFI_N));

static void init_adc(){

    hal::adc1.init({
            {hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T28_5}
        },{

            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T13_5},
            {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T13_5},
            {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T13_5},  
            {hal::AdcChannelSelection::TEMP, hal::AdcSampleCycles::T28_5},  

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

    static constexpr auto DEADTIME_NANOS = MOS_1C840L_500MA_BEST_DEADTIME_NS;
    // static constexpr auto MOS_1C840L_100MA_BEST_DEADTIME = 350ns;
    timer.bdtr().init({DEADTIME_NANOS});
    // timer.init_bdtr(MOS_1C840L_100MA_BEST_DEADTIME);

    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__A7_B0_B1,
        // .count_freq = hal::NearestFreq(CHOPPER_FREQ * 2),
        .count_freq = hal::timer::ArrAndPsc{TIMER_ARR_VALUE,1-1},
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

    timer.oc<4>().init({
        .oc_mode = hal::TimerOcMode::ActiveAboveCvr,
        .cvr_sync_en = EN,
        .valid_level = HIGH,
        .out_en = DISEN
    });
    // timer.oc<4>().cvr() = timer.arr() - ADC_SAMPLE_TRIM_CC_VALUE - 5;
    timer.oc<4>().cvr() = timer.arr() - 8;

    timer.oc<4>().enable_output(EN);

    #define TIM_INST TIM1

    auto get_timer_tick = []() -> TimerTick{
        auto * inst = TIM_INST;
        if(timer.is_up_counting()){
            return TimerTick{
                .counter_value = static_cast<uint16_t>(inst->CNT),
                .is_up_counting = true
            };
        }else{
            return TimerTick{
                .counter_value = static_cast<uint16_t>(inst->CNT),
                .is_up_counting = false
            };
        }
    };

    auto set_uvw_dutycycle = [&](const UvwCoord<iq16> & dutycycle){
        auto * inst = TIM_INST;
        const uint16_t arr = static_cast<uint16_t>(inst->ATRLR);
        const uint16_t half_arr = arr >> 1;
        
        auto convert = [&](const iq16 channel_dutycycle) -> uint16_t{
            return uint16_t(int32_t((channel_dutycycle.to_bits() * arr) >> 16) + half_arr);
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


    auto get_uvw_current_u12x3 = [&] -> std::array<uint32_t, 3>{
        auto * inst = ADC1;
        return {
            static_cast<uint32_t>(inst->IDATAR1),
            static_cast<uint32_t>(inst->IDATAR2),
            static_cast<uint32_t>(inst->IDATAR3)
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

    #if 0
    auto mag_encoder_ = MotorProfile::MagEncoder{
        &spi,
        spi.allocate_cs_pin(&mag_encoder_cs_pin_)
            .unwrap()
    };
    #endif


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


    // #endregion
    #if 0
    constexpr auto FLUX_SENSORLESS_OB_COEFFS = dsp::motor_ctl::NonlinearFluxObserver::Config{
        .fs = FOC_FREQ,
        .phase_inductance_mh = MotorProfile::PHASE_INDUCTANCE_MH,
        .phase_resistance_ohm = MotorProfile::PHASE_RESISTANCE_OHM,

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
        .phase_inductance_mh = float(MotorProfile::PHASE_INDUCTANCE_MH),
        .phase_resistance_ohm = float(MotorProfile::PHASE_RESISTANCE_OHM),

        // .observer_gain = 10.1023, // [rad/s]
        .observer_gain = 0.1073, // [rad/s]
        .pm_flux_linkage = 0.000877, // [V / (rad/s)]
        // .pm_flux_linkage = 0.01277, // [V / (rad/s)]
    }.try_into_precomputed().unwrap();

    auto nlf_ob_ = dsp::motor_ctl::NonlinearFluxObserver(FLUX_SENSORLESS_OB_COEFFS);
    #endif


    #if 0
    static constexpr iq16 f_para = 1 - MotorProfile::PHASE_RESISTANCE_OHM / (MotorProfile::PHASE_INDUCTANCE_MH * FOC_FREQ / 1000);
    static constexpr iq16 g_para = 1 / (MotorProfile::PHASE_INDUCTANCE_MH * FOC_FREQ / 1000);
    [[maybe_unused]] auto smo_sensorless_ob_ = dsp::motor_ctl::SlideModeObserver{
        dsp::motor_ctl::SlideModeObserver::Config{
            .f_para = f_para,
            .g_para = g_para,
            .kslide = 0.22_iq16,
            .kslf = 0.6_iq16,
        }
    };
    #endif

    using Nltd2o = NonlinearTrackingDifferentiator<iq16, 2>;


    [[maybe_unused]] static constexpr auto command_shaper_ = Nltd2o::Config{
        .fs = FOC_FREQ,
        .r = 587.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 40
    }.try_into_precomputed().unwrap();





    OpFlags op_flags_;
    op_flags_.initial_dc_calibrate = true;
    FnSwitches fn_switches_;

    auto p_all_state_ = std::make_unique<AllState>();
    AllState & all_state_ = *p_all_state_;
    all_state_.reset();

    [[maybe_unused]] auto current_sense = [&](AllState & state){
        const auto uvw_current_u12x3 = get_uvw_current_u12x3();
        
        auto & dc_cal_state = state.dc_calibrate_state;

        if(op_flags_.initial_dc_calibrate == true){
            dc_cal_state.uvw_current_bits_offset_acc = {
                std::get<0>(dc_cal_state.uvw_current_bits_offset_acc) + static_cast<uint32_t>(std::get<0>(uvw_current_u12x3)),
                std::get<1>(dc_cal_state.uvw_current_bits_offset_acc) + static_cast<uint32_t>(std::get<1>(uvw_current_u12x3)),
                std::get<2>(dc_cal_state.uvw_current_bits_offset_acc) + static_cast<uint32_t>(std::get<2>(uvw_current_u12x3))
            };
            dc_cal_state.dc_cal_cnt++;
            if(dc_cal_state.dc_cal_cnt >= DC_CAL_TIMES){
                dc_cal_state.uvw_current_bits_offset = {
                    static_cast<uint16_t>(std::get<0>(dc_cal_state.uvw_current_bits_offset_acc) / int32_t(DC_CAL_TIMES)),
                    static_cast<uint16_t>(std::get<1>(dc_cal_state.uvw_current_bits_offset_acc) / int32_t(DC_CAL_TIMES)),
                    static_cast<uint16_t>(std::get<2>(dc_cal_state.uvw_current_bits_offset_acc) / int32_t(DC_CAL_TIMES))
                };
                op_flags_.initial_dc_calibrate = false;
            }

            {
                const auto LOW_HFI_VOLT = iq20(0.6_iq16);
                const auto [s,c] = dsp::DFT32_BIN1_SINCOS_TABLE[dc_cal_state.dc_cal_cnt & 0x1f];
                const auto alphabeta_volt_gen = AlphaBetaCoord<iq16>{
                    .alpha = iq15::from_bits(s.to_bits()) * LOW_HFI_VOLT,
                    .beta = iq15::from_bits(c.to_bits()) * LOW_HFI_VOLT
                };

                set_uvw_dutycycle(SVM(alphabeta_volt_gen * INV_BUS_VOLT * iq16(1.5)));
            }
            return;
        }


        //#region 电流传感


        state.uvw_curr_raw = UvwCoord<iq20>{
            .u = CURRENT_AMPS_PER_ADC_LSB *
                (int32_t(uvw_current_u12x3[0]) - int32_t(dc_cal_state.uvw_current_bits_offset[0])),
            .v = CURRENT_AMPS_PER_ADC_LSB *
                (int32_t(uvw_current_u12x3[1]) - int32_t(dc_cal_state.uvw_current_bits_offset[1])),
            .w = CURRENT_AMPS_PER_ADC_LSB *
                (int32_t(uvw_current_u12x3[2]) - int32_t(dc_cal_state.uvw_current_bits_offset[2])),
        };

        state.u_disconn_dbs.add_sample(judge_is_disconn(state.uvw_curr_raw[0], state.uvw_curr_raw[1] + state.uvw_curr_raw[2]));
        state.v_disconn_dbs.add_sample(judge_is_disconn(state.uvw_curr_raw[1], state.uvw_curr_raw[0] + state.uvw_curr_raw[2]));
        
        state.unblance_curr_abs_lp = lpf_50hz(
            state.unblance_curr_abs_lp,
            math::abs(state.uvw_curr_raw.numeric_sum())
        );


        state.alphabeta_curr_raw = AlphaBetaCoord<iq20>::from_uvw(state.uvw_curr_raw);
        state.alphabeta_curr_fastlp[0] = lpf_1000hz(state.alphabeta_curr_fastlp[0], state.alphabeta_curr_raw[0]);
        state.alphabeta_curr_fastlp[1] = lpf_1000hz(state.alphabeta_curr_fastlp[1], state.alphabeta_curr_raw[1]);
        //#endregion
    };
    [[maybe_unused]] auto mechanical_loop = [&](AllState & state){
        //#region 力矩转电流

        [[maybe_unused]] static constexpr iq20 TORQUE_2_CURRENT_RATIO = 1_iq20;
        [[maybe_unused]] static constexpr iq20 CURRENT_LIMIT = 1.2_iq16;


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
        //#endregion
    };


    [[maybe_unused]] auto ob_loop = [&](AllState & state){
        // leso_state_var_ = leso.iterate(leso_state_var_, state.rotor_rotation_state_var.x1, state.torque_curr_cmd);
        // leso_state_var_ = leso.iterate(leso_state_var_, state.rotor_rotation_state_var.x2, state.torque_curr_cmd);
        // leso_state_var_ = leso.iterate(leso_state_var_, math::fixed_downcast<16>(state.rotor_rotation_state_var.x1), state.torque_curr_cmd);
    };

    uq32 hybrid_ratio = 0;

    [[maybe_unused]] auto torque_loop = [&](AllState & state){


        //#region 位置提取

        [[maybe_unused]] const auto now_secs = clock::seconds();



        const bool is_openloop = true;
        // const bool is_openloop = false;
        const bool run_hfi = true;
        // const bool run_hfi = false;


        [[maybe_unused]] const bool run_smo = true;

        [[maybe_unused]] auto iterate_hfi_angle = [&](const Angular<uq32> hfi_pll_angle){
            const auto now_hfi_lap_angle = hfi_pll_angle.cast_inner<uq16>();

            const auto hfi_diff_angle = (now_hfi_lap_angle.cast_inner<iq16>()
                - state.hfi_lap_angle.cast_inner<iq16>()).signed_normalized();

            state.hfi_lap_angle = now_hfi_lap_angle;
            state.hfi_multilap_angle = state.hfi_multilap_angle + hfi_diff_angle;
            // rotor_rotation_ltd_.iterate(state.rotor_rotation_state_var, {state.hfi_multilap_angle.to_turns(), 0});

            state.hfi_elec_angle = make_angular_from_turns(uq32((state.hfi_multilap_angle / 2).unsigned_normalized().to_turns()));
            state.hfi_elec_angle = state.hfi_elec_angle + make_angular_from_turns(0.8125_uq32);
            state.hfi_elec_angle = state.hfi_elec_angle + make_angular_from_turns(0.5_uq32);
            // state.hfi_elec_angle = state.hfi_elec_angle + make_angular_from_turns(uq32(iq32(state.hfi_pll_state.angluar_speed_integral.to_turns() * uq32(1.0 / 3500))));
        };

        [[maybe_unused]] auto iterate_encoder_angle = [&](){
            #if 0
            const auto angle_packet = mag_encoder_.update().examine();

            const auto encoder_lap_turns = angle_packet.parse().unwrap().to_turns();
            const auto prev_encoder_lapturns = state.encoder_lap_turns;
            const auto encoder_diff_turns = uq32_wrapped_diff(prev_encoder_lapturns, encoder_lap_turns);

            state.encoder_lap_turns = encoder_lap_turns;
            state.encoder_multilap_turns = state.encoder_multilap_turns + encoder_diff_turns;
            rotor_rotation_ltd_.iterate(
                state.rotor_rotation_state_var,
                {fixed_downcast<16>(state.encoder_multilap_turns), 0}
            );

            state.sensed_elec_angle = Angular<uq32>::from_turns(
                math::pu_to_uq32(encoder_lap_turns * MotorProfile::POLE_PAIRS)
                + MotorProfile::SENSORED_ELEC_ANGLE_BASE.to_turns()
            );
            state.selected_elec_angle = state.sensed_elec_angle;
            #endif
        };


        static constexpr auto PLL_COEFFS = dsp::PllCoeffs::from_fsfc(FOC_FREQ, 75, 2.0_iq16);

        if(is_openloop){
            constexpr uq32 DT = uq32::from_rcp(FOC_FREQ);
            const auto speed = -0.40_iq16;
            const auto openloop_elec_angle = make_angular_from_turns(
                state.openloop_elec_angle.to_turns() 
                + uq32(DT * speed * MotorProfile::POLE_PAIRS)
            );


            state.openloop_elec_angle = openloop_elec_angle;
        }


        if(run_hfi){
            // static constexpr auto PLL_COEFFS = dsp::PllCoeffs::from_fsfc(FOC_FREQ, 30);
            // static constexpr auto PLL_COEFFS = dsp::PllCoeffs::from_fskpki(FOC_FREQ, 2000, 30000);


            // PLL_COEFFS.iterate(state.hfi_pll_state, {
            //     iq16(state.hfi_bin2_real_response * 30),
            //     iq16(state.hfi_bin2_imag_response * 30)
            // });
            auto bin2_real_response = state.hfi_bin2_real_response;
            auto bin2_imag_response = state.hfi_bin2_imag_response;

            if(math::abs(state.hfi_pll_state.angluar_speed_integral.to_turns()) > 10){
                bin2_real_response -= state.hfi_bin2_real_response_slowlp;
                bin2_imag_response -= state.hfi_bin2_imag_response_slowlp;
            }

            PLL_COEFFS.iterate(state.hfi_pll_state, {
                iq16((bin2_real_response) * 30),
                iq16((bin2_imag_response) * 30)
            });


            iterate_hfi_angle(state.hfi_pll_state.angle);
            // state.observer_elec_angle= state.hfi_pll_state.angle + Angular<uq32>::QUARTER;

        }else{
            //TODO: take off side effects
        }



        state.observer_elec_angle = make_angular_from_turns(
            state.obs_pll_state.angle.to_turns()
            //  + uq32(state.obs_pll_state.angluar_speed.to_turns() * uq32(1.0 /2000))
        );
        
        {
            // hybrid_ratio = uq32(CLAMP(math::abs(state.hfi_pll_state.angluar_speed.to_turns()) * uq32(1.0 / 50) - 3, 0_iq16, 0.99_iq16));
            hybrid_ratio = 0.0_uq32;
            state.hybrid_elec_angle = lerp_pu_angle(
                state.hfi_elec_angle,
                make_angular_from_turns(state.observer_elec_angle.to_turns()),
                hybrid_ratio
            );
        }

        // state.selected_elec_angle = state.openloop_elec_angle;
        state.selected_elec_angle = state.hfi_elec_angle;
        // state.selected_elec_angle = state.hybrid_elec_angle;
        // state.selected_elec_angle = state.hfi_pll_state.angle + Angular<uq32>::QUARTER;

        const auto elec_sincos = math::Rotation2<iq16>::from_angle(state.selected_elec_angle);

        //#endregion

        const auto dq_curr_raw = state.alphabeta_curr_raw.to_dq(elec_sincos);
        state.dq_curr_fastlp[0] = lpf_1000hz(state.dq_curr_fastlp[0], dq_curr_raw[0]);
        state.dq_curr_fastlp[1] = lpf_1000hz(state.dq_curr_fastlp[1], dq_curr_raw[1]);

        
        DqCoord<iq20> dq_curr_setp = DqCoord<iq20>{0.0_iq20, state.torque_curr_cmd};
        
        const bool do_mtpa = false;

        //TODO mtpa
        if(do_mtpa){
        }


        #if 1
        [[maybe_unused]] auto generate_alpha_beta_volt_by_spin_hfi = [&]{
            // static constexpr uint32_t MASK = (HFI_N) - 1;


            const auto [_s_bin1,_c_bin1] = DFT32_BIN1_SINCOS_TABLE[state.hfi_idx];
            const auto s_bin1 = iq15::from_bits(_s_bin1.to_bits());
            const auto c_bin1 = iq15::from_bits(_c_bin1.to_bits());

            const auto hfi_response = dot2v2(
                state.alphabeta_curr_raw.alpha, c_bin1,
                state.alphabeta_curr_raw.beta, s_bin1
            );

            #if 1
            if(state.hfi_is_neg_samp){
                const auto di = hfi_response - state.hfi_response;
                state.hfi_response = hfi_response;

                state.hfi_buffer[state.hfi_idx] = di;

                if(state.hfi_idx >= HFI_N){
                    state.hfi_idx = 0;

                    for(size_t i = 0; i < 1; i++){
                        const auto buffer_view = std::span(state.hfi_buffer);

                        // const auto hfi_bin0_real_response = dft32_bin0<20>(buffer_view);
                        // state.hfi_bin0_real_response = hfi_bin0_real_response;

                        // const auto [hfi_bin1_real_response, hfi_bin1_imag_response] = dft32_bin1<20>(buffer_view);
                        // state.hfi_bin1_real_response = lpf_allpass(state.hfi_bin1_real_response, hfi_bin1_real_response);
                        // state.hfi_bin1_imag_response = lpf_allpass(state.hfi_bin1_imag_response, hfi_bin1_imag_response);

                        auto [hfi_bin2_real_response_raw, hfi_bin2_imag_response_raw] = dft32_bin2<20>(buffer_view);

                        //2207
                        // hfi_bin2_real_response -= -0.040_iq20;
                        // hfi_bin2_imag_response -= 0.019_iq20;


                        state.hfi_bin2_real_response = lpf_allpass(state.hfi_bin2_real_response, hfi_bin2_real_response_raw);
                        state.hfi_bin2_imag_response = lpf_allpass(state.hfi_bin2_imag_response, hfi_bin2_imag_response_raw);
                        
                        state.hfi_bin2_real_response_slowlp = lpf_100hz(state.hfi_bin2_real_response_slowlp, hfi_bin2_real_response_raw);
                        state.hfi_bin2_imag_response_slowlp = lpf_100hz(state.hfi_bin2_imag_response_slowlp, hfi_bin2_imag_response_raw);
                    }
                }else{
                    state.hfi_idx += 1;
                }
            }else{
                state.hfi_response = hfi_response;
            }
            #endif

            if(state.hfi_is_neg_samp){
                state.hfi_is_neg_samp = false;
                return AlphaBetaCoord<iq20>{
                    .alpha = HFI_VOLT * c_bin1,
                    .beta = HFI_VOLT * s_bin1,
                };
            }else{
                state.hfi_is_neg_samp = true;
                return AlphaBetaCoord<iq20>{
                    .alpha = (-HFI_VOLT) * c_bin1,
                    .beta = (-HFI_VOLT) * s_bin1,
                };
            }
        };
        #endif


        iq20 d_volt_ff = 0;
        iq20 q_volt_ff = 0;

        const bool cross_decoupling_enabled = fn_switches_.cross_decoupling_en;

        if(cross_decoupling_enabled){
        // if(true){
            const bool is_speed_stable = true;
            // const auto omega = state.rotor_rotation_state_var.x2 * is_speed_stable * iq16(TAU);
            const auto omega = state.hfi_pll_state.angluar_speed_integral.to_radians() * is_speed_stable;
            d_volt_ff -= MotorProfile::PHASE_INDUCTANCE_MH * dq_curr_raw.q * omega;
            q_volt_ff += MotorProfile::PHASE_INDUCTANCE_MH * dq_curr_raw.d * omega;
        }

        const bool bemf_decoupling_enabled = fn_switches_.bemf_decoupling_en;
        if(bemf_decoupling_enabled){
            const bool is_speed_stable = true;
            const auto omega = state.hfi_pll_state.angluar_speed_integral.to_radians() * is_speed_stable;
           q_volt_ff += MotorProfile::FLUX_LINKAGE * omega;
        }

        state.dq_volt_ff.d = d_volt_ff;
        state.dq_volt_ff.q = q_volt_ff;


        
        const iq20 kp = PI_CONTROLLER_COEFFS.kp;
        const iq20 ki_discrete = PI_CONTROLLER_COEFFS.ki_discrete;

        const iq20 d_curr_err = (dq_curr_setp[0] - dq_curr_raw[0]);
        const iq20 q_curr_err = (dq_curr_setp[1] - dq_curr_raw[1]);

        state.dq_volt_integral[0] = state.dq_volt_integral[0] + d_curr_err * ki_discrete;
        state.dq_volt_integral[1] = state.dq_volt_integral[1] + q_curr_err * ki_discrete;
        
        
        DqCoord<iq20> dq_volt_gen = Zero;
        
        #if 1
        const iq20 volt_limit_radius = CTRL_MODU_DEPTH_LIMIT * BUS_VOLT;
        {
            const iq20 d_volt_limit = volt_limit_radius;
            const iq20 d_volt_unclamped = d_volt_ff + d_curr_err * kp + state.dq_volt_integral[0];
            dq_volt_gen[0] = CLAMP2(d_volt_unclamped, d_volt_limit);
            state.dq_volt_integral[0] += (dq_volt_gen[0] - d_volt_unclamped);
        }
        
        {
            const iq20 q_volt_limit = heightleg(volt_limit_radius, dq_volt_gen[0]);
            // const iq20 q_volt_limit = volt_limit_radius;
            const iq20 q_volt_unclamped = q_volt_ff + q_curr_err * kp + state.dq_volt_integral[1];
            dq_volt_gen[1] = CLAMP2(q_volt_unclamped, q_volt_limit);
            state.dq_volt_integral[1] += (dq_volt_gen[1] - q_volt_unclamped);
        }
        #endif

        const auto inv_busbar_volt = INV_BUS_VOLT;
        const auto inv_busbar_volt_3by2 = (inv_busbar_volt * 3) >> 1;

        auto dq_dutycycle_gen = dq_volt_gen * inv_busbar_volt_3by2;

        auto alphabeta_dutycycle_gen = dq_dutycycle_gen.to_alphabeta(elec_sincos);

        // const bool deadtime_comp_en = false;
        const bool deadtime_comp_en = fn_switches_.deadtime_compensate_en;

        if(deadtime_comp_en){
            // https://www.zhihu.com/question/270446098/answer/3215795384
            // 《SVPWM逆变器死区补偿的研究与实现》 魏凯

            const auto uvw_curr_fastlp = state.alphabeta_curr_fastlp.to_uvw();
            static constexpr auto ONE_BY_3 = uq32(1.0 / 3.0);
            static constexpr auto ONE_BY_SQRT3 = uq32(1.0 / 1.73205080757);
            static constexpr auto DEADTIME_COMP_DUTYCYCLE = uq32(
                (DEADTIME_NANOS.count() * FOC_FREQ * 1e-9)
            );

            const auto alpha_sign = (
                +2 * math::sign(uvw_curr_fastlp.u) 
                - math::sign(uvw_curr_fastlp.v) 
                - math::sign(uvw_curr_fastlp.w)
            ) * ONE_BY_3;

            const auto beta_sign = (
                + math::sign(uvw_curr_fastlp.v) 
                - math::sign(uvw_curr_fastlp.w)
            ) * ONE_BY_SQRT3;

            const auto deadtime_comp_alphabeta_dutycycle = AlphaBetaCoord<iq20>{
                alpha_sign * DEADTIME_COMP_DUTYCYCLE,
                beta_sign * DEADTIME_COMP_DUTYCYCLE
            };


            #define DEADTIME_LPF_FN lpf_100hz
            // #define DEADTIME_LPF_FN lpf_allpass
            state.deadtime_comp_alphabeta_dutycycle[0] = DEADTIME_LPF_FN(state.deadtime_comp_alphabeta_dutycycle[0], deadtime_comp_alphabeta_dutycycle[0]);
            state.deadtime_comp_alphabeta_dutycycle[1] = DEADTIME_LPF_FN(state.deadtime_comp_alphabeta_dutycycle[1], deadtime_comp_alphabeta_dutycycle[1]);

            alphabeta_dutycycle_gen.alpha -= state.deadtime_comp_alphabeta_dutycycle.alpha;
            alphabeta_dutycycle_gen.beta -= state.deadtime_comp_alphabeta_dutycycle.beta;
        }


        const bool spin_hfi_enabled = true;
        // const bool spin_hfi_enabled = false;

        if(spin_hfi_enabled){
            state.spinhfi_alphabeta_volt_gen  = generate_alpha_beta_volt_by_spin_hfi();
            alphabeta_dutycycle_gen = alphabeta_dutycycle_gen + 
                state.spinhfi_alphabeta_volt_gen * inv_busbar_volt;
        }

        auto uvw_dutycycle_gen = SVM(
            alphabeta_dutycycle_gen
        );



        set_uvw_dutycycle(uvw_dutycycle_gen);


        state.busbar_curr_raw = (state.uvw_curr_raw.dot(uvw_dutycycle_gen));
        state.busbar_curr = lpf_10hz(state.busbar_curr, state.busbar_curr_raw);
        state.uvw_dutycycle_gen = uvw_dutycycle_gen;

        state.dq_curr_raw = dq_curr_raw;
        state.dq_volt_gen = dq_volt_gen;

        state.alphabeta_volt_gen = alphabeta_dutycycle_gen * BUS_VOLT;

        // flux_sensorless_ob.update(alphabeta_volt_gen, alphabeta_curr_raw);
        // smo_sensorless_ob_.update({state.alphabeta_curr_raw, state.alphabeta_volt_gen});
        // lbg_sensorless_ob.update({alphabeta_curr_raw, alphabeta_volt_gen});
        
        if(0){

            nlf_ob_.update(state.alphabeta_curr_raw, state.alphabeta_volt_gen);
            PLL_COEFFS.iterate(state.obs_pll_state, {
                iq16((nlf_ob_.state().eta_mf[0]) * 3),
                iq16(-(nlf_ob_.state().eta_mf[1]) * 3)
            });
        }
    };

    auto jeoc_isr = [&]{
        auto  & state = all_state_;
        timming_watch_pin_.set_high();


        state.isr_entry_tick = get_timer_tick();

        current_sense(state);
        if(not op_flags_.initial_dc_calibrate){
            mechanical_loop(state);
            torque_loop(state);
        }

        state.isr_exit_tick = get_timer_tick();
        // state.isr_elapsed_ticks.bits = state.isr_entry_tick.bits - state.isr_entry_tick.bits;
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

    set_uvw_dutycycle(UvwCoord<iq16>::HALF);
    timer.start();

    clock::delay(2ms);

    const auto temp_trimer = hal::TemperatureTrimer::load();
    iq16 die_celsius_ = 0;


    [[maybe_unused]] auto poll_repl_activity = [&]{
        [[maybe_unused]] static repl::ReplServer repl_server{&DBG_UART, &DBG_UART};
        repl_server.enable_echo(DISEN);

        [[maybe_unused]] static const auto list = script::make_list(
            "root",
            script::make_function(StringView("tc"), [&](iq20 torque_curr_cmd){
                if(math::abs(torque_curr_cmd) > 10) return;
                all_state_.torque_curr_cmd = torque_curr_cmd;
            }),

            script::make_function(StringView("dce"), [&](const bool en){
                fn_switches_.deadtime_compensate_en = en;
            }),

            script::make_function(StringView("cde"), [&](const bool en){
                fn_switches_.cross_decoupling_en = en;
            })
        );

        repl_server.invoke(list);
    };
    
    all_state_.torque_curr_cmd = 0.0_iq20;

    while(true){
        poll_repl_activity();
        [[maybe_unused]] const auto now_secs = clock::seconds();

        die_celsius_ = lpf_10hz(die_celsius_, temp_trimer.parse_u12(ADC1->IDATAR4));

        // const auto targ = 40_iq20 + iq16(math::sin(3 * now_secs)) * 253.4_iq20;
        // all_state_.torque_curr_cmd += 0.00005_iq20 * CLAMP2((targ - all_state_.hfi_pll_state.angluar_speed.to_turns()), 10.0_iq20);
        // all_state_.torque_curr_cmd = CLAMP2(all_state_.torque_curr_cmd, 2.4_iq20);
        // all_state_.torque_curr_cmd = 0.0_iq20;

        // {
            const auto offset = all_state_.hfi_bin0_real_response;
            const auto amp = math::mag(all_state_.hfi_bin2_real_response, all_state_.hfi_bin2_imag_response) * 2;
            static constexpr auto factor = (int)(iq12(FOC_FREQ) / iq12(HFI_VOLT));
            [[maybe_unused]] const auto lq_est_uh = int(1000000 / factor) / (offset - amp);
            [[maybe_unused]] const auto ld_est_uh = int(1000000 / factor) / (offset + amp);
        // }

        // {
            // [[maybe_unused]] const auto mag_volt = math::mag(all_state_.alphabeta_volt_gen[0], all_state_.alphabeta_volt_gen[1]);
            // [[maybe_unused]] const auto mag_curr = math::mag(all_state_.alphabeta_curr_raw[0], all_state_.alphabeta_curr_raw[1]);
        // }

        if(true)DEBUG_PRINTLN(
            // all_state_.hfi_response,
            // die_celsius_,
            // s, c, 
            // pwm_u_.cvr(),
            // pwm_v_.cvr(),
            // pwm_w_.cvr(),
            // all_state_.uvw_curr_raw.u,
            // all_state_.uvw_curr_raw.v + all_state_.uvw_curr_raw.w,
            // all_state_.uvw_curr_raw.v,
            // all_state_.uvw_curr_raw.w,
            // all_state_.uvw_curr_slowlp.u,
            // all_state_.uvw_curr_slowlp.v + all_state_.uvw_curr_slowlp.w,
            // u_disconn_dbs.count,
            // v_disconn_dbs.count,
            // judge_is_disconn(all_state_.uvw_curr_slowlp.u,all_state_.uvw_curr_slowlp.v + all_state_.uvw_curr_slowlp.w),
            // judge_is_disconn(all_state_.uvw_curr_slowlp.v,all_state_.uvw_curr_slowlp.u + all_state_.uvw_curr_slowlp.w),
            // all_state_.alphabeta_volt_gen.alpha,
            // all_state_.alphabeta_volt_gen.beta,
            // all_state_.d_volt_gen,
            // all_state_.q_volt_gen,
            // flux_sensorless_ob.angle().to_turns(),
            // flux_sensorless_ob.all_state_().flux_state_mf[0],
            // flux_sensorless_ob.all_state_().flux_state_mf[1],
            // flux_sensorless_ob.all_state_().v_alphabeta_last[0],
            // flux_sensorless_ob.all_state_().v_alphabeta_last[1],
            // all_state_.torque_curr_cmd,
            // all_state_.uvw_curr_raw,
            // all_state_.dq_volt_gen,
            // all_state_.selected_elec_angle.to_turns(),
            // all_state_.dq_curr_raw.d,
            // all_state_.dq_volt_gen.q,
            // all_state_.dq_curr_raw.q,

            // all_state_.spinhfi_alphabeta_volt_gen,
            // all_state_.hfi_bin1_real_response,
            // all_state_.hfi_bin2_real_response,
            // all_state_.hfi_bin2_imag_response,
            // all_state_.hfi_bin1_real_response,
            // all_state_.hfi_bin1_imag_response,
            // dsp::dot2v2(
            all_state_.hfi_bin2_real_response, 
            // all_state_.hfi_bin1_real_response,
            all_state_.hfi_bin2_imag_response, 
            // all_state_.hfi_bin1_imag_response
            // ),
            // all_state_.hfi_bin2_real_response_slowlp,
            // all_state_.hfi_bin2_imag_response_slowlp,
            // all_state_.dq_volt_gen.q,
            // all_state_.alphabeta_curr_raw.alpha,
            // all_state_.alphabeta_curr_raw.beta,
            // all_state_.dq_volt_ff,
            // lq_est_uh,
            // ld_est_uh,
            // mag_volt,
            // mag_curr,
            // mag_volt / mag_curr,
            // all_state_.alphabeta_curr_raw.length(),
            // all_state_.dq_volt_gen.length(),
            // all_state_.dq_volt_gen.length() / all_state_.alphabeta_curr_raw.length(),
            // all_state_.alphabeta_volt_gen.length(),
            // math::atan2(all_state_.hfi_bin1_imag_response,
            //     all_state_.hfi_bin1_real_response),

            // math::atan2(all_state_.hfi_bin2_imag_response,
            //     all_state_.hfi_bin2_real_response) / 2,
            // all_state_.openloop_elec_angle.to_turns(),
            // all_state_.hfi_pll_state.angle.to_turns(),
            // all_state_.hfi_elec_angle.to_turns(),
            // nlf_ob_.state().eta_mf[0],
            // nlf_ob_.state().eta_mf[1],
            // all_state_.hfi_pll_state.angluar_speed_integral.to_turns(),
            // all_state_.obs_pll_state.angluar_speed_integral.to_turns(),
            all_state_.hfi_elec_angle.to_turns(),
            // all_state_.observer_elec_angle.to_turns(),
            // all_state_.hybrid_elec_angle.to_turns(),
            // hybrid_ratio,

            // offset / (offset * offset - amp * amp),

            // iq12(offset) * int(1000000/ factor),
            // iq12(amp) * int(1000000/ factor),
            // all_state_.hfi_elec_angle.to_turns(),
            // hfi_bin2_angle.to_turns(),

            // hfi_idx,
            // all_state_.alphabeta_curr_raw[0],
            // all_state_.alphabeta_curr_raw[1],
            // all_state_.deadtime_comp_alphabeta_dutycycle,
            // all_state_.uvw_dutycycle_gen,
            // all_state_.busbar_curr,
            // hfi_bin2_angle.to_turns()
            // all_state_.hfi_bin2_imag_response,
            // all_state_.hfi_bin2_real_response
            // full_arr,
            // all_state_.isr_elapsed_ticks.count(),
            
            // die_celsius_,
            // flux_sensorless_ob.angle().to_turns(),
            // math::atan2pu(all_state_.alphabeta_curr_raw[0], all_state_.alphabeta_curr_raw[1]),
            // all_state_.alphabeta_curr_raw[0] / all_state_.alphabeta_curr_raw[1],

            tmrticks_to_us(all_state_.isr_entry_tick),
            tmrticks_to_us(all_state_.isr_exit_tick)
            // tmrticks_to_us(all_state_.isr_exit_tick) - tmrticks_to_us(all_state_.isr_entry_tick)
            // all_state_.isr_entry_tick.counter_value,
            // all_state_.isr_exit_tick.counter_value
            
            // tmrticks_diff_to_us(all_state_.isr_entry_tick, all_state_.isr_exit_tick)

            // uint16_t(TIM_INST->ATRLR)
            // timer.oc<4>().cvr(),
            // timer.arr()
            // all_state_.isr_elapsed_ticks.count()
            // all_state_.alphabeta_volt_gen,
            // all_state_.alphabeta_curr_raw
            // all_state_.uvw_curr_raw.u,
            // all_state_.uvw_curr_raw.v,
            // all_state_.uvw_curr_raw.w
        );
        // clock::delay(4ms);

        poll_blink_service();
        // toggle_red_led();
        // repl_service_poller();
        // clock::delay(2ms);
    }

}

