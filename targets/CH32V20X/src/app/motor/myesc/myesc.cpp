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



#include "drivers/gatedrv/DRV832X/DRV8323h.hpp"

#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"

#include "dsp/motor_ctrl/sensorless/slide_mode_observer.hpp"
#include "dsp/motor_ctrl/sensorless/luenberger_observer.hpp"
#include "dsp/motor_ctrl/sensorless/nonlinear_flux_observer.hpp"

#include "dsp/controller/adrc/nonlinear/nltd2o.hpp"
#include "dsp/controller/adrc/linear/ltd2o.hpp"
#include "dsp/filter/firstorder/lpf.hpp"
#include "dsp/filter/butterworth/band.hpp"

#include "motor_dsp/dsp_lpf.hpp"
#include "motor_dsp/dsp_vec.hpp"
#include "motor_dsp/dsp_fft32.hpp"

#include "motor_dsp/dsp_pi.hpp"

#include "motor_config.hpp"

#include "core/sdk.hpp"

#include "core/math/clamp.hpp"




using namespace ymd;

using namespace ymd::drivers;

using namespace ymd::dsp::adrc;
using namespace ymd::myesc;




#define DBG_UART hal::usart2


enum class [[nodiscard]] DemoTrajPattern:uint8_t{
    Stop,
    Sine,
    Saw,
    Stairs
};

__no_inline static constexpr TrajState
calc_demo_traj(const uq16 now_secs, const DemoTrajPattern demo_pattern){
    switch(demo_pattern){
        case DemoTrajPattern::Stop:{
            return {0, 0};
        }
        case DemoTrajPattern::Sine:{
            constexpr auto omega = 1_iq16;
            constexpr auto side_amplitude = 1.4_iq16;

            const auto [s,c] = math::sincos(omega * now_secs);
            return {
                side_amplitude * iq16(s),
                side_amplitude * omega * iq16(c)
            };
        }
        case DemoTrajPattern::Saw:{
            // const auto [s,c] = math::sincos(omega * now_secs);

            constexpr auto freq = 0.5_iq16;
            constexpr auto amplitude = 5.0_iq16;
            constexpr auto slew_rate = amplitude * freq;
            return {-iq16(math::frac(now_secs * freq)) * amplitude, -slew_rate};
        }
        case DemoTrajPattern::Stairs:{
            constexpr auto freq = 0.3_iq16;
            constexpr size_t num_steps = 6;
            constexpr auto half_amplitude = 0.4_iq16;
            constexpr auto step = half_amplitude * 2/ num_steps;
            const auto s = iq16(math::sinpu(now_secs * freq));
            return {(math::floor(s * (num_steps / 2)) * step), 0};
        }
    }
    //unreachable
    return {0, 0};
};



template<size_t FC, size_t Q>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, int32_t> lpf_specified_fc(
    const math::fixed<Q, int32_t> x_state,
    const math::fixed<Q, int32_t> x_new
){
    constexpr auto ALPHA = dsp::calc_lpf_alpha_uq32(FOC_FREQ, FC).unwrap();
    return lpf_1o(x_state, x_new, ALPHA);
}

constexpr auto ALPHA_100HZ = dsp::calc_lpf_alpha_uq32(FOC_FREQ, 100).unwrap();
constexpr auto ALPHA_10HZ = dsp::calc_lpf_alpha_uq32(FOC_FREQ, 10).unwrap();
constexpr auto ALPHA_1HZ = dsp::calc_lpf_alpha_uq32(FOC_FREQ, 1).unwrap();
constexpr auto ALPHA_01HZ = dsp::calc_lpf_alpha_uq32(FOC_FREQ * 10, 1).unwrap();

#define DEF_GEN_LPF_FUNC(fc)\
template<size_t Q>\
__always_inline __attribute__((const, optimize( "-Ofast" )))\
static constexpr math::fixed<Q, int32_t> lpf_##fc##hz(\
    math::fixed<Q, int32_t> x_state,\
    const math::fixed<Q, int32_t> x_new\
){\
    return lpf_specified_fc<fc>(x_state, x_new);\
}\


DEF_GEN_LPF_FUNC(10)
DEF_GEN_LPF_FUNC(100)
DEF_GEN_LPF_FUNC(50)
DEF_GEN_LPF_FUNC(1000)
DEF_GEN_LPF_FUNC(2000)
DEF_GEN_LPF_FUNC(500)

template<size_t Q>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, int32_t> lpf_allpass(
    math::fixed<Q, int32_t> x_state, const math::fixed<Q, int32_t> x_new
){
    return x_new;
}



static constexpr size_t OBSERVER_PLL_FC = 65;
static constexpr size_t HFI_PLL_FC = 65;

static constexpr auto OBSERVER_PLL_COEFFS = 
    dsp::PllCoeffs::from_fsfc(FOC_FREQ, OBSERVER_PLL_FC, 2.0_iq16);

static constexpr auto HFI_PLL_COEFFS = 
    dsp::PllCoeffs::from_fsfc(FOC_FREQ, HFI_PLL_FC, 2.0_iq16);



static constexpr uq32 TSAMPLE = uq32::from_rcp(FOC_FREQ);
static constexpr iq20 HW_TORQUE_CURRENT_LIMIT = 
    std::min(
        CURRENT_HALFSCALE_AMPS, 
        iq20(BUSBAR_VOLT) * uq32(0.666) / MotorProfile::PHASE_RESISTANCE_OHM
    ) * uq32(0.8);

static constexpr auto CURRENT_REGULATOR_CFG = dsp::LrSeriesCurrentRegulatorConfig{
    .fs = FOC_FREQ,
    .fc = MotorProfile::PREFERD_CURRENT_CUTOFF_FREQ,
    .phase_inductance_mh = MotorProfile::PHASE_INDUCTANCE_MH,
    .phase_resistance_ohm = MotorProfile::PHASE_RESISTANCE_OHM,
};


static constexpr auto PI_CONTROLLER_COEFFS = CURRENT_REGULATOR_CFG.try_into_precomputed().unwrap();

using Ltd2o = dsp::adrc::LinearTrackingDifferentiator<iq16, 2>;

static constexpr size_t HIGHFREQ_ENCODER_LTD2O_R = 1200;
[[maybe_unused]] static constexpr auto HIGHFREQ_ENCODER_LTD_2O = Ltd2o::try_from({
    .fs = FOC_FREQ, 
    .r = HIGHFREQ_ENCODER_LTD2O_R
}).unwrap();



static constexpr int32_t CURVE_X2_LIMIT = 10;

static constexpr int32_t E1_LIMIT = 100;
static constexpr int32_t E2_LIMIT = 1000;


[[maybe_unused]] static constexpr auto CURVE_NLTD_FHAN = FhanPrecomputed<iq16>::from({
    .r = 204.5_iq16,
    .h = 0.003_iq16,
});




static constexpr bool judge_is_disconn(const iq20 meas, const iq20 ref){
    const auto abs_ref = math::abs(ref);
    if(abs_ref < CURRENT_NOISE_STDVAR) return false;
    const auto abs_meas = math::abs(meas);
    return (abs_meas * 30 < abs_ref) and (abs_meas < CURRENT_NOISE_STDVAR);
}




static constexpr iq16 _tmrticks_to_us(const int32_t counter_value){
    static constexpr uint32_t factor = (1ull << 32) * (1.0 / 144);
    return iq16::from_bits(int32_t((int64_t(counter_value) * factor) >> 16));
}

static constexpr iq16 tmrticks_to_us(const TimerTick tick){
    int32_t counter_value = int32_t(tick.counter_value);
    if(tick.is_up_counting) counter_value = TIMER_ARR_VALUE + counter_value;
    else counter_value = TIMER_ARR_VALUE - counter_value;
    return _tmrticks_to_us(counter_value);
}

[[nodiscard]] static constexpr size_t 
warp_index(const size_t x, const size_t len){
    size_t next = x;
    if(next > len) 
        next -= len;
    return next;
};


#define TIM_INST TIM1
#define ADC_INST ADC1

__no_inline static void setup_adc(){

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

static void setup_timer(){
    auto & timer = hal::timer1;
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

    timer.bdtr().init({DEADTIME_NANOS});
    timer.enable_arr_sync(EN);

    timer.oc<1>().init(Default);
    timer.oc<2>().init(Default);
    timer.oc<3>().init(Default);    
    timer.oc<4>().init({
        .oc_mode = hal::TimerOcMode::ActiveAboveCvr,
        .cvr_sync_en = EN,
        .valid_level = HIGH,
        .out_en = EN
    });


    {
        timer.oc<1>().cvr() = TIMER_ARR_VALUE >> 1;
        timer.oc<2>().cvr() = TIMER_ARR_VALUE >> 1;
        timer.oc<3>().cvr() = TIMER_ARR_VALUE >> 1;
        timer.oc<4>().cvr() = TIMER_ARR_VALUE - 8;
    }


    timer.ocn<1>().init(Default);
    timer.ocn<2>().init(Default);
    timer.ocn<3>().init(Default);
}



static TimerTick get_timer_tick() {
    auto * inst = TIM_INST;
    if(lld::timer_is_up_counting(inst)){
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

static void set_uvw_dutycycle(UvwCoord<iq16> dutycycle, const bool phase_invert_en){
    auto * inst = TIM_INST;
    
    const uint16_t half_arr = TIMER_ARR_VALUE >> 1;
    if(phase_invert_en) std::swap(dutycycle.v, dutycycle.w);
    
    auto convert = [&](const iq16 channel_dutycycle) -> uint16_t{
        return uint16_t(int32_t((channel_dutycycle.to_bits() * TIMER_ARR_VALUE) >> 16) + half_arr);
    };

    
    inst->CH1CVR = convert(dutycycle.template get<0>());
    inst->CH2CVR = convert(dutycycle.template get<1>());
    inst->CH3CVR = convert(dutycycle.template get<2>());
};

static std::array<int32_t, 3> get_adc_uvw_bvalue(){
    auto * inst = ADC1;
    return {
        static_cast<int32_t>(inst->IDATAR1),
        static_cast<int32_t>(inst->IDATAR2),
        static_cast<int32_t>(inst->IDATAR3)
    };
};


static void stop_pwm(){
    TIM_INST->CTLR1 &= (uint16_t)(~((uint16_t)TIM_CEN));
    TIM_INST->BDTR &= (uint16_t)(~((uint16_t)TIM_MOE));
};

static void start_pwm(){
    TIM_INST->CTLR1 |= (uint16_t)TIM_CEN;
    TIM_INST->BDTR |= (uint16_t)TIM_MOE;
};



static void setup_drv8323(){


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
}


[[maybe_unused]] __no_inline 
static void process_current_sense(
    AllState & state, 
    FnSwitches fn_switches, 
    std::array<int32_t, 3U> uvw_bvalue
){
    
    auto & dc_state = state.dc_calibrate_state;

    state.uvw_curr_raw = UvwCoord<iq20>{
        .u = CURRENT_AMPS_PER_ADC_LSB *
            (int32_t(uvw_bvalue[0]) - int32_t(dc_state.uvw_bvalue_offset[0])),
        .v = CURRENT_AMPS_PER_ADC_LSB *
            (int32_t(uvw_bvalue[1]) - int32_t(dc_state.uvw_bvalue_offset[1])),
        .w = CURRENT_AMPS_PER_ADC_LSB *
            (int32_t(uvw_bvalue[2]) - int32_t(dc_state.uvw_bvalue_offset[2])),
    };

    if(fn_switches.phase_invert_en){
        std::swap(state.uvw_curr_raw[1], state.uvw_curr_raw[2]);
    }

    state.uvw_curr_fastlp[0] = lpf_1000hz(state.uvw_curr_fastlp[0], state.uvw_curr_raw[0]);
    state.uvw_curr_fastlp[1] = lpf_1000hz(state.uvw_curr_fastlp[1], state.uvw_curr_raw[1]);
    state.uvw_curr_fastlp[2] = lpf_1000hz(state.uvw_curr_fastlp[2], state.uvw_curr_raw[2]);

    // state.u_disconn_dbs.add_sample(judge_is_disconn(state.uvw_curr_raw[0], state.uvw_curr_raw[1] + state.uvw_curr_raw[2]));
    // state.v_disconn_dbs.add_sample(judge_is_disconn(state.uvw_curr_raw[1], state.uvw_curr_raw[0] + state.uvw_curr_raw[2]));
    
    // state.unblance_curr_abs_lp = lpf_50hz(
    //     state.unblance_curr_abs_lp,
    //     math::abs(state.uvw_curr_raw.numeric_sum())
    // );


    state.busbar_curr_raw = (state.uvw_curr_raw.dot(state.uvw_dutycycle_genout));
    state.busbar_curr_lp = lpf_50hz(state.busbar_curr_lp, state.busbar_curr_raw);
    
    state.alphabeta_curr_raw = AlphaBetaCoord<iq20>::from_uvw(state.uvw_curr_raw);
    //#endregion
};




[[maybe_unused]] __no_inline 
static void process_position_sense(AllState & state, const FnSwitches fn_switches){



    // if(1){//sensorless observer
    if(false){//sensorless observer
        constexpr auto L = MotorProfile::PHASE_INDUCTANCE_MH * uq32(0.001);
        constexpr auto R = MotorProfile::PHASE_RESISTANCE_OHM;
        constexpr auto lambda = MotorProfile::FLUX_LINKAGE;
        auto & flux_ob_state = state.flux_ob_state;

        const auto iaib = state.alphabeta_curr_raw;
        const auto vavb = state.alphabeta_volt_final;
        [[maybe_unused]] const auto prev_iaib = state.prev_alphabeta_curr_raw;


        const auto R_iaib = R * iaib;


        #if 0
        // MXLEMMING

        #if 0
        auto x1 = flux_ob_state.x1;
        auto x2 = flux_ob_state.x2;

        x1 += (vavb[0] - R_iaib[0]) * TSAMPLE - L * (iaib[0] - prev_iaib[0]);
        x2 += (vavb[1] - R_iaib[1]) * TSAMPLE - L * (iaib[1] - prev_iaib[1]);

        x1 = math::clamp2(x1, lambda);
        x2 = math::clamp2(x2, lambda);

        flux_ob_state.x1_slowlp = lpf_10hz(flux_ob_state.x1_slowlp, x1);
        flux_ob_state.x2_slowlp = lpf_10hz(flux_ob_state.x2_slowlp, x2);
        flux_ob_state.x1_slowhp = hpf_1o(flux_ob_state.x1_slowhp, x1, flux_ob_state.x1, ALPHA_10HZ);
        flux_ob_state.x2_slowhp = hpf_1o(flux_ob_state.x2_slowhp, x2, flux_ob_state.x2, ALPHA_10HZ);

        flux_ob_state.x1 = x1;
        flux_ob_state.x2 = x2;
        #else

        // 保存上一时刻的输入
        const auto x1_prev = flux_ob_state.x1;
        const auto x2_prev = flux_ob_state.x2;

        auto x1 = x1_prev;
        auto x2 = x2_prev;

        const auto x1_delta = (vavb[0] - R_iaib[0]) * TSAMPLE - L * (iaib[0] - prev_iaib[0]);
        const auto x2_delta = (vavb[1] - R_iaib[1]) * TSAMPLE - L * (iaib[1] - prev_iaib[1]);

        x1 = math::clamp2(x1 + x1_delta, lambda);
        x2 = math::clamp2(x2 + x2_delta, lambda);

        flux_ob_state.x1_slowlp = lpf_10hz(flux_ob_state.x1_slowlp, x1);
        flux_ob_state.x2_slowlp = lpf_10hz(flux_ob_state.x2_slowlp, x2);

        // 使用上一时刻的输入值
        // flux_ob_state.x1_slowhp = hpf_1o_delta(
        //     flux_ob_state.x1_slowhp,  // y_prev
        //     x1_delta,
        //     uq32::from_bits(~ALPHA_10HZ.to_bits())
        // );

        // flux_ob_state.x1_slowhp = leaky_1o(
        //     flux_ob_state.x1_slowhp, x1_delta *FOC_FREQ, uq32::from_bits(~ALPHA_10HZ.to_bits())
        // );

        // flux_ob_state.x1_slowhp = x1 - x1_prev;
        // flux_ob_state.x2_slowhp = hpf_1o(
        //     flux_ob_state.x2_slowhp,
        //     x2,
        //     x2_prev,                  // x[n-1] 上一时刻输入 ← 关键！
        //     ALPHA_10HZ
        // );

        flux_ob_state.x1 = x1;
        flux_ob_state.x2 = x2;
        #endif

        auto pll_input_sine = x2;
        auto pll_input_cosine = x1;

        if(math::abs(state.observer_pll_state.angluar_speed.to_turns()) > 30){
            pll_input_sine -= flux_ob_state.x2_slowlp;
            pll_input_cosine -= flux_ob_state.x1_slowlp;
        }
    
        OBSERVER_PLL_COEFFS.iterate(state.observer_pll_state, {
            iq16(pll_input_sine) * 80,
            iq16(pll_input_cosine) * 80
        });
        #else
        // ORTEGA
        
        const auto L_iaib = L * iaib;
        
        // https://zhuanlan.zhihu.com/p/887911569 反馈增益的设置是定为20000-100000之间调整
        [[maybe_unused]] constexpr auto gamma_half = 70000;
        constexpr auto gamma_half_dt = uq16(float(gamma_half) / FOC_FREQ);

        const auto lem1 = flux_ob_state.x1 - L_iaib[0];
        const auto lem2 = flux_ob_state.x2 - L_iaib[1];
        auto abs_lem = math::square(lem1) + math::square(lem2);
        // auto abs_lem = math::mag(lem1, lem2);
        auto err = (lambda - abs_lem);

        if (err > 0) {
            err = 0;
        }

        auto x1_delta = (vavb[0] - R_iaib[0]) * TSAMPLE + (lem1) * err * gamma_half_dt;
        auto x2_delta = (vavb[1] - R_iaib[1]) * TSAMPLE + (lem2) * err * gamma_half_dt;

        auto x1 = flux_ob_state.x1;
        auto x2 = flux_ob_state.x2;
        
        x1 += x1_delta;
        x2 += x2_delta;

        flux_ob_state.x1_slowlp = lpf_10hz(flux_ob_state.x1_slowlp, lem1);
        flux_ob_state.x2_slowlp = lpf_10hz(flux_ob_state.x2_slowlp, lem2);

        flux_ob_state.x1 = x1;
        flux_ob_state.x2 = x2;

        flux_ob_state.lem1 = lem1;
        flux_ob_state.lem2 = lem2;

        flux_ob_state.flux_err = err;
        flux_ob_state.abs_lem = abs_lem;

        auto pll_input_sine = x2 - L_iaib[1];
        auto pll_input_cosine = x1 - L_iaib[0];

        if(math::abs(state.observer_pll_state.angluar_speed.to_turns()) > 30){
            pll_input_sine -= flux_ob_state.x2_slowlp;
            pll_input_cosine -= flux_ob_state.x1_slowlp;
        }
    
        OBSERVER_PLL_COEFFS.iterate(state.observer_pll_state, {
            iq16(pll_input_sine) * 80, 
            iq16(pll_input_cosine) * 80
        });
        // OBSERVER_PLL_COEFFS.iterate(state.observer_pll_state, {
        //     iq16(40 * (flux_ob_state.x2 - lem2)),
        //     iq16(40 * (flux_ob_state.x1 - lem1))
        // });

        #endif

        state.prev_alphabeta_curr_raw = iaib;

        state.observer_elec_angle = make_angular_from_turns(
            state.observer_pll_state.angle.to_turns()
        );
    }

    {
        
        auto iterate_openloop_angle = [&]{


            state.openloop_elec_speed = 5.80_iq16;

            state.openloop_elec_angle = make_angular_from_turns(
                state.openloop_elec_angle.to_turns() 
                + uq32(TSAMPLE * state.openloop_elec_speed)
            );
        };

        {
            const auto encoder_offset_base = make_angular_from_turns(0.053571_uq32);
            const auto pole_pairs = MotorProfile::POLE_PAIRS;

            const auto encoder_mech_angle = make_angular_from_turns(
                uq32::from_bits(uint32_t(state.encoder_abs_turns64.to_bits())));

            auto & state_2o = state.encoder_state_2o;

            auto & x1_now = state_2o.x1;
            auto & x2_now = state_2o.x2;

            const auto x1_now_q20 = math::comp_downcast<20>(x1_now);

            const iq20 e1 = math::clamp2(fixed_downcast<20>(state.encoder_abs_turns64) - x1_now_q20, E1_LIMIT);
            const iq20 e2 = - x2_now;


            x1_now = x1_now + static_cast<iiq32>(extended_mul(x2_now, TSAMPLE));
            x2_now = x2_now + math::comp_downcast<20>(
                extended_mul(iq20(2 * e2), HIGHFREQ_ENCODER_LTD_2O.r_by_fs) 
                + extended_mul(iq20(e1), HIGHFREQ_ENCODER_LTD_2O.r2_by_fs));

            state.sensed_elec_angle = (encoder_mech_angle + encoder_offset_base) * pole_pairs;

            const auto mech_speed = state.encoder_state_2o.x2;
            state.sensed_elec_speed = mech_speed * pole_pairs;

        }


        std::tie(state.elec_angle, state.elec_speed) = [&] -> std::tuple<Angular<uq32>, iq16>{
            switch(fn_switches.elec_angle_source){
                case ElecAngleSource::Openloop:{
                    iterate_openloop_angle();
                    const auto elec_angle = state.openloop_elec_angle;
                    const auto elec_speed = state.openloop_elec_speed;
                    return {elec_angle, elec_speed};
                }

                case ElecAngleSource::Observer:{
                    const auto elec_angle = state.observer_elec_angle;

                    //use integral instead
                    const auto elec_speed = state.observer_pll_state.angluar_speed_integral.to_turns();
                    return {elec_angle, elec_speed};
                }
                
                
                case ElecAngleSource::Hfi:{
                    const auto elec_angle = state.hfi_elec_angle;
                    const auto elec_speed = iq16(0);
                    return {elec_angle, elec_speed};
                }
                
                case ElecAngleSource::MagEncoder:
                case ElecAngleSource::AbzEncoder:
                    //TODO
                    const auto elec_angle = state.sensed_elec_angle;
                    const auto elec_speed = state.sensed_elec_speed;
                    return {elec_angle, elec_speed};
            }
            __builtin_unreachable();
        }();

    }
};


[[maybe_unused]] __no_inline 
static void process_traj_generate(AllState & state, [[maybe_unused]] FnSwitches fn_switches){
    #if 1
    [[maybe_unused]] const auto now_secs = clock::seconds();

    static constexpr auto demo_pattern = 
        // DemoTrajPattern::Sine
        // DemoTrajPattern::Stop
        // DemoTrajPattern::Stairs
        DemoTrajPattern::Saw
    ;

    state.traj_state = calc_demo_traj(now_secs, demo_pattern);
    #endif

}



static constexpr size_t pow2(const size_t x){
    return __builtin_ctz(x);
}

static_assert(pow2(1) == 0);
static_assert(pow2(2) == 1);
static_assert(pow2(16) == 4);

[[maybe_unused]] __no_inline 
static void process_traj_shape(AllState & state, FnSwitches fn_switches){

    const auto traj_smooth_method = fn_switches.traj_smooth_method;
    const bool traj_frontend_smooth_en = traj_smooth_method != TrajSmoothMethod::Disabled;
    
    if(traj_frontend_smooth_en){
        //启用此项 为轨迹规划器提供准确的前馈速度 使得规划输出与原始位置信号几乎零迟滞

        const bool use_input_x2 = traj_smooth_method == TrajSmoothMethod::UseX1AndX2;

        const auto & traj_state = state.traj_state;
        auto & now_state = state.traj_smooth_state;

        auto & x1_now = now_state.x1;
        auto & x2_now = now_state.x2;

        const auto x1_now_q16 = math::comp_downcast<16>(x1_now);

        auto & x1_ref = traj_state.x1;
        auto x2_ref = (use_input_x2 ? traj_state.x2 : iq16(0));

        const iq20 e1 = math::clamp2(x1_ref - x1_now_q16, E1_LIMIT);
        const iq20 e2 = math::clamp2(x2_ref - x2_now, E2_LIMIT);

        x1_now = x1_now + static_cast<iiq32>(extended_mul(x2_now, TSAMPLE));
        x2_now = x2_now + math::comp_downcast<20>(
            extended_mul(iq20(2 * e2), HIGHFREQ_ENCODER_LTD_2O.r_by_fs) 
            + extended_mul(iq20(e1), HIGHFREQ_ENCODER_LTD_2O.r2_by_fs));
    }else{
        state.traj_smooth_state.x1 = iiq32::from_bits((int64_t)state.traj_state.x1.to_bits() << 16);

        //无前馈速度
        state.traj_smooth_state.x2 = 0;
    }

    {
        //基于fhan的梯形速度规划
        const auto & traj_state = state.traj_smooth_state;
        auto & curve_state = state.curve_state;

        const iq16 x1_now = math::fixed_downcast<16>(curve_state.x1);
        const auto x2_now = curve_state.x2;

        const iq20 x2_traj = traj_state.x2;

        const bool do_x1_comp = traj_smooth_method == TrajSmoothMethod::UseX1AndZero;

        //使用终值定理得到无静差时补偿量为x2 * 2/r
        const iq16 x1_comp = do_x1_comp ? iq16(x2_traj * uq32(2.0 / HIGHFREQ_ENCODER_LTD2O_R)) : iq16(0);
        const iq16 x1_traj = math::fixed_downcast<16>(traj_state.x1) + x1_comp;
        
        const iq16 e1 = math::clamp2(x1_traj - x1_now, E1_LIMIT);
        const iq20 e2 = math::clamp2(x2_traj - x2_now, E2_LIMIT);

        const auto u = CURVE_NLTD_FHAN({e1, iq16(e2)});

        const auto next_x1 = curve_state.x1 + iiq32(extended_mul(curve_state.x2, TSAMPLE));
        const auto next_x2 = math::clamp2(curve_state.x2 + iq20(u) * TSAMPLE, CURVE_X2_LIMIT);

        const auto delta_x2 = next_x2 - curve_state.x2;

        curve_state.x1 = next_x1;
        curve_state.x2 = next_x2;
        curve_state.x3 = math::closer_to_zero(iq20(u), lpf_2000hz(curve_state.x3, delta_x2 * FOC_FREQ));
        // curve_state.x3 = delta_x2 * FOC_FREQ;
        curve_state.u = u;
    }
}


[[maybe_unused]] __no_inline 
static void process_mechanical_loop(AllState & state, FnSwitches fn_switches){
    //#region 力矩转电流

    [[maybe_unused]] static constexpr iq20 TORQUE_2_CURRENT_RATIO = 1_iq20;



    const auto now_x2 = state.encoder_state_2o.x2;

    constexpr bool is_x2_noisy = true;

    auto now_x1 = math::fixed_downcast<16>(state.encoder_state_2o.x1);

    if(not is_x2_noisy){
        now_x1 += (now_x2 * uq32(2.0 / HIGHFREQ_ENCODER_LTD2O_R));
    }

    const auto curve_x1 = math::fixed_downcast<16>(state.curve_state.x1);
    const auto curve_x2 = state.curve_state.x2;

    constexpr auto torque_curr_step_limit = iq20(0.02);
    constexpr auto torque_curr_limit = iq20(6.0);

    const auto loop_wiring = fn_switches.loop_wiring;

    const iq20 ka = 0.001_iq20;
    const auto x3comp_torque_curr = ka * state.curve_state.x3;

    const iq20 kf = 0.01_iq20;
    const auto x2comp_torque_curr = kf * state.curve_state.x2;


    state.torque_curr_x3comp = x3comp_torque_curr;

    switch(loop_wiring){
        case LoopWiring::SeriesPi:{
            const iq20 kpp = 65.0_iq20;
            // const iq20 kp = 1.2_iq20;
            constexpr iq20 kp = 0.4_iq20;
            constexpr iq20 ki = 8.66_iq20;
            constexpr auto ki_discrete = ki / FOC_FREQ;

            const iq20 x2_ref = compmul_clamp2(iq20(curve_x1 - now_x1), kpp, E2_LIMIT);

            state.pi_x2_ref = x2_ref;
            // state.pi_x2_ref = curve_x1 - now_x1;
            // const iq20 x2_ref = 0;
            const iq20 e2 = math::clamp2((x2_ref + curve_x2) - now_x2, E2_LIMIT);
            state.pi_e2 = e2;

            auto desired_torque_curr_cmd = state.torque_curr_integral 
                + compmul(kp, e2)
                + x2comp_torque_curr
                + x3comp_torque_curr
            ;

            auto next_torque_curr_cmd = math::clamp2(desired_torque_curr_cmd, torque_curr_limit);
            next_torque_curr_cmd = (3 * state.torque_curr_cmd + next_torque_curr_cmd) >> 2;
            state.torque_curr_cmd = math::step_to(state.torque_curr_cmd, next_torque_curr_cmd, torque_curr_step_limit);

            auto torque_curr_integral_delta = 
                e2 * ki_discrete
                // + (state.torque_curr_cmd - desired_torque_curr_cmd)
            ;
            state.torque_curr_integral += math::clamp2(
                torque_curr_integral_delta,
                torque_curr_step_limit
            );
            break;
        }

        case LoopWiring::Mit:{
            const iq20 kp = 12.7_iq16;
            const iq20 kd = 0.26_iq16;

            const iq20 e1 = iq20(math::clamp2(curve_x1 - now_x1, E1_LIMIT));
            const iq20 e2 = iq20(math::clamp2(curve_x2 - now_x2, E2_LIMIT));
            
            iq20 torque_curr_cmd = (kp * e1) + (kd * e2)
                + x2comp_torque_curr
                + x3comp_torque_curr
            ;
            torque_curr_cmd = math::clamp2(torque_curr_cmd, torque_curr_limit);

            state.torque_curr_integral = 0;
            state.torque_curr_cmd = math::step_to(state.torque_curr_cmd, torque_curr_cmd, torque_curr_step_limit);
            break;
        }



        case LoopWiring::SeriesAdrc:{
            #if 0
            //rubbish
            const iq20 kpp = 10.0_iq20;
            constexpr iq20 b0 = 200.1_iq20;
            constexpr iq20 inv_b0 = 1 / b0;
            constexpr size_t wc = 36;
            constexpr size_t wo = 3 * wc;
            constexpr uq32 wo2t = (wo * wo) * TSAMPLE;

            const auto x2_ref = math::clamp2(iq16(kpp * (curve_x1 - now_x1)), E2_LIMIT) + curve_x2;

            auto & eso_state = state.speed_eso_state;

            const auto iq = math::clamp2(iq20((wc * (x2_ref - eso_state.speed_est) - eso_state.f_est) * inv_b0), torque_curr_limit);
            eso_state.speed_est += (eso_state.f_est - 2 * wo * (eso_state.speed_est - now_x2) + b0 * iq) * TSAMPLE;
            eso_state.f_est += -(eso_state.speed_est - now_x2) * wo2t;

            state.torque_curr_cmd = iq;
            #endif
            break;
        }
    }
};




[[maybe_unused]] __no_inline static void process_disturb_ob_loop(AllState & state, const FnSwitches fn_switches){

};


#if 0

[[maybe_unused]] __no_inline static 
void process_harmonic_suppression(
    AllState & state, 
    const FnSwitches fn_switches,
    iq16 elec_speed
){
    // if(fn_switches.current_harmonic_suppression_en){
    // if(true){
    if(false){
        // Analysis and Control of Current Harmonic in IPMSMField-Oriented Control System
        // DOI：10.1109/TPEL.2022.3155243

        bool is_traditional = true;
        const auto dq_curr_ref = state.dq_curr_ref;
        state.alphabeta_curr_ref = dq_curr_ref.rotate(elec_sincos);
        state.uvw_curr_ref = state.alphabeta_curr_ref.to_uvw();

        const auto elec_sincos_5x = math::Rotation2<iq31>::from_angle(elec_angle * 5u);

        #if 0
        const auto elec_sincos_6x = elec_sincos_5x.rotate(elec_sincos);
        const auto elec_sincos_7x = elec_sincos_6x.rotate(elec_sincos);
        #else

        const auto elec_sincos_6x = math::Rotation2<iq31>::from_angle(elec_angle * 6u);
        const auto elec_sincos_7x = math::Rotation2<iq31>::from_angle(elec_angle * 7u);
        #endif

        const auto alphabeta_curr_harmonic = (state.alphabeta_curr_raw - state.alphabeta_curr_ref);

        const auto dq5_curr_raw = alphabeta_curr_harmonic.inv_rotate(elec_sincos_5x);
        const auto dq7_curr_raw = alphabeta_curr_harmonic.inv_rotate(elec_sincos_7x);

        state.dq5_curr_raw = dq5_curr_raw;
        state.dq7_curr_raw = dq7_curr_raw;

        // const auto lpf_alpha = ALPHA_1HZ;
        const auto lpf_alpha = ALPHA_01HZ;

        state.dq5_curr_lp[0] = lpf_1o(state.dq5_curr_lp[0], dq5_curr_raw[0], lpf_alpha);
        state.dq5_curr_lp[1] = lpf_1o(state.dq5_curr_lp[1], dq5_curr_raw[1], lpf_alpha);

        state.dq7_curr_lp[0] = lpf_1o(state.dq7_curr_lp[0], dq7_curr_raw[0], lpf_alpha);
        state.dq7_curr_lp[1] = lpf_1o(state.dq7_curr_lp[1], dq7_curr_raw[1], lpf_alpha);

        const auto is5c = state.dq5_curr_lp.q;
        const auto is5s = state.dq5_curr_lp.d;

        const auto is7c = state.dq7_curr_lp.q;
        const auto is7s = state.dq7_curr_lp.d;

        const auto pi_weak_factor = uq32(0.992);
        // const auto pi_weak_factor = uq32(0.099);


        const iq20 kp = PI_CONTROLLER_COEFFS.kp * pi_weak_factor;
        // const iq20 kp = 0;
        const iq20 ki_discrete = PI_CONTROLLER_COEFFS.ki_discrete * pi_weak_factor;
        
        auto & harmonic_state = state.harmonic_state;

        if(is_traditional){
            auto & vs5c_integral = harmonic_state.integrals[0];
            auto & vs5s_integral = harmonic_state.integrals[1];
            auto & vs7c_integral = harmonic_state.integrals[2];
            auto & vs7s_integral = harmonic_state.integrals[3];

            #define RUN_PI_CONTROLLER(v, i)\
            const iq20 i##_err = (0-i);\
            v##_integral = math::clamp2(v##_integral + i##_err * ki_discrete, iq20(0.7));\
            [[maybe_unused]] auto v = v##_integral + kp * i##_err;\

            RUN_PI_CONTROLLER(vs5c, is5c)
            RUN_PI_CONTROLLER(vs5s, is5s)
            RUN_PI_CONTROLLER(vs7c, is7c)
            RUN_PI_CONTROLLER(vs7s, is7s)

            [[maybe_unused]] const auto c_6 = elec_sincos_6x.cosine();
            [[maybe_unused]] const auto c_n6 = c_6;
            [[maybe_unused]] const auto s_6 = elec_sincos_6x.sine();
            [[maybe_unused]] const auto s_n6 = -s_6;

            //Fig 5
            const auto delta_vd6_in = 
                // vs5c * c_n6 - vs5s * s_n6 
                + vs7c * c_6 - vs7s * s_6
            ;
            const auto delta_vq6_in = 
                // vs5c * s_n6 + vs5s * c_n6 
                + vs7c * s_6 + vs7s * c_6
            ;

            harmonic_state.delta_vd6_in = delta_vd6_in;
            harmonic_state.delta_vq6_in = delta_vq6_in;

            #undef RUN_PI_CONTROLLER
        }else{
            // [25]
            const auto id6c = is7c + is5c;
            const auto id6s = is7s + is5s;

            // [26]
            const auto iq6c = is7c - is5c;
            const auto iq6s = is7s - is5s;

            harmonic_state.id6c = id6c;
            harmonic_state.id6s = id6s;
            harmonic_state.iq6c = iq6c;
            harmonic_state.iq6s = iq6s;

            const auto phi_td = uq32((12 * elec_speed) * TSAMPLE);
            // const auto phi_td = uq32(0);

            const auto [phi_s, phi_c] = math::sincospu(phi_td);

            auto & vd6c_integral = harmonic_state.integrals[0];
            auto & vd6s_integral = harmonic_state.integrals[1];
            auto & vq6c_integral = harmonic_state.integrals[2];
            auto & vq6s_integral = harmonic_state.integrals[3];

            #define RUN_PI_CONTROLLER(v, i)\
            const iq20 i##_err = (0-i);\
            v##_integral = math::clamp2(v##_integral + i##_err * ki_discrete, iq20(0.4));\
            auto v = v##_integral + kp * i##_err;\

            RUN_PI_CONTROLLER(vd6c, id6c)
            RUN_PI_CONTROLLER(vd6s, id6s)
            RUN_PI_CONTROLLER(vq6c, iq6c)
            RUN_PI_CONTROLLER(vq6s, iq6s)

            constexpr auto L = MotorProfile::PHASE_INDUCTANCE_MH * uq32(0.001);
            // constexpr auto R = MotorProfile::PHASE_RESISTANCE_OHM;
            const auto dec_factor = kp * phi_s - 6 * elec_omega * L;

            vd6c += id6s * dec_factor;
            vd6s -= id6c * dec_factor;
            vq6c += iq6s * dec_factor;
            vq6s -= iq6c * dec_factor;

            #undef RUN_PI_CONTROLLER

            // [36]
            const auto vd6c_in = vd6c * phi_c - vd6s * phi_s;
            const auto vd6s_in = vd6s * phi_c + vd6c * phi_s;

            // [37]
            const auto vq6c_in = vq6c * phi_c - vq6s * phi_s;
            const auto vq6s_in = vq6s * phi_c + vq6c * phi_s;

            // const auto [s6, c6] = elec_sincos_6x;
            const auto s6 = elec_sincos_6x.sine();
            const auto c6 = elec_sincos_6x.cosine();

            // [38]
            const auto delta_vd6_in = vd6c_in * c6 - vd6s_in * s6;
            const auto delta_vq6_in = vq6c_in * s6 + vq6s_in * c6;

            harmonic_state.delta_vd6_in = delta_vd6_in;
            harmonic_state.delta_vq6_in = delta_vq6_in;
        }

    }
}

#endif


__no_inline static 
void process_current_loop(
    AllState & state, 
    const FnSwitches fn_switches, 
    Angular<uq32> elec_angle,
    iq16 elec_speed,
    iq20 tc_ref
){
    const auto elec_omega = elec_speed * iq16(TAU);

    const auto elec_sincos = math::Rotation2<iq31>::from_angle(elec_angle);

    //#endregion


    {
        const auto dq_curr_raw = state.alphabeta_curr_raw.inv_rotate(elec_sincos);
        state.dq_curr_fastlp[0] = lpf_2000hz(state.dq_curr_fastlp[0], dq_curr_raw[0]);
        state.dq_curr_fastlp[1] = lpf_2000hz(state.dq_curr_fastlp[1], dq_curr_raw[1]);
        state.dq_curr_raw = dq_curr_raw;
    }

    {

        // enum class [[nodiscard]] DqCurrentSource:uint8_t{
        //     IdAlwaysZero,
        //     IdAlwaysZeroRamped,
        //     Independent,
        //     IndependentRamped,
        //     Torque,
        // };



        iq20 d_curr_ref = 0;
        iq20 q_curr_ref = tc_ref;
        


        const bool do_mtpa = fn_switches.mtpa_en;
        const bool do_mtpv = fn_switches.mtpv_en;


        constexpr auto ld_lq_diff = MotorProfile::Q_AXIS_INDUCTANCE_MH - MotorProfile::D_AXIS_INDUCTANCE_MH;

        constexpr auto lambda = 1000 * MotorProfile::FLUX_LINKAGE;
        constexpr auto ld_lq_diff_by_lambda = ld_lq_diff / lambda;

        if(do_mtpa){
            #if 0
            // \frac{\left(l-\sqrt{l^{2}+8\cdot\left(d\right)^{2}x^{2}}\right)}{4d}
            constexpr auto inv_4_ld_lq_diff = 1 / (4 * ld_lq_diff);
            constexpr auto ld_lq_diff_x2sqr2 = ld_lq_diff * iq20(2.828);

            const auto d_curr_set_tmp = (lambda - math::mag(lambda, ld_lq_diff_x2sqr2 * (tc_ref))) * inv_4_ld_lq_diff;

			const auto q_curr_set_tmp = [&] -> iq20{
                // if(math::abs(d_curr_set_tmp) > math::abs(tc_ref)) return 0;
                iq20 abs_iq = math::heightleg(tc_ref, d_curr_set_tmp);
                return (tc_ref < 0) ? -abs_iq : abs_iq;
            }();

            #else
            //use taylor
            // -\frac{d}{l}x^{2}
            // x-\frac{d^{2}}{2l^{2}}x^{3}
            constexpr auto taylor_d_curr_coeff = -ld_lq_diff / lambda;
            constexpr auto taylor_iq_coeff = 2 * math::square(ld_lq_diff / lambda);

            const auto approx_tc_ref = iq12(tc_ref);
            const auto approx_tc_ref_2 = math::square(approx_tc_ref);
            const auto approx_tc_ref_3 = approx_tc_ref_2 * (approx_tc_ref);

            d_curr_ref = taylor_d_curr_coeff * approx_tc_ref_2 ;
            q_curr_ref = tc_ref - iq20(taylor_iq_coeff * (approx_tc_ref_3));
            #endif

            state.mtpa_d_curr = d_curr_ref;
            state.mtpa_q_curr = q_curr_ref;
            
        }

        //TODO mtpv
        if(do_mtpv){
        }

        state.dq_curr_ref.d = d_curr_ref;
        state.dq_curr_ref.q = q_curr_ref;

        state.backcalc_torque_curr = q_curr_ref * (1 + ld_lq_diff_by_lambda * d_curr_ref);

    }

    {
        iq20 d_volt_decouple = 0;
        iq20 q_volt_decouple = 0;

        const bool is_speed_stable = true;
        const auto omega = elec_omega * is_speed_stable;
        
        const auto phase_ind = MotorProfile::PHASE_INDUCTANCE_MH * uq32(0.001);
        
        
        const bool cross_decoupling_enabled = fn_switches.cross_decoupling_en;

        if(cross_decoupling_enabled){
            d_volt_decouple -= phase_ind * state.dq_curr_raw.q * omega;
            q_volt_decouple += phase_ind * state.dq_curr_raw.d * omega;
        }

        const bool bemf_decoupling_enabled = fn_switches.bemf_decoupling_en;

        if(bemf_decoupling_enabled){
            q_volt_decouple += MotorProfile::FLUX_LINKAGE * omega;
        }

        state.dq_volt_decouple.d = d_volt_decouple;
        state.dq_volt_decouple.q = q_volt_decouple;
    }



    {//pi
        const iq20 kp = PI_CONTROLLER_COEFFS.kp;
        const iq20 ki_discrete = PI_CONTROLLER_COEFFS.ki_discrete;

        auto dq_volt_ff = 
            state.dq_volt_decouple
            // +
        ;

        if(fn_switches.current_harmonic_suppression_en){
            dq_volt_ff.d += state.harmonic_state.delta_vd6_in;
            dq_volt_ff.q += state.harmonic_state.delta_vq6_in;
        }
        
        const iq20 d_curr_err = (state.dq_curr_ref[0] - state.dq_curr_raw[0]);
        const iq20 q_curr_err = (state.dq_curr_ref[1] - state.dq_curr_raw[1]);

        state.dq_volt_integral[0] = state.dq_volt_integral[0] + d_curr_err * ki_discrete;
        state.dq_volt_integral[1] = state.dq_volt_integral[1] + q_curr_err * ki_discrete;
        
        
        // https://davidmolony.github.io/MESC_Firmware/operation/CONTROL.html
        // Vd preferencing circle limiter
        // There may be reasons to prefer Vd to Vq. 
        // One such reason is that when we apply the field weakening, 
        // we need to ensure there is sufficient voltage available to generate the d axis current. 
        // Since the field weakening current is typically set lower than the torque current, 
        // a linear implementation of the circle limiter will result in reduced d axis current 
        // with increasing throttle

        DqCoord<iq20> dq_volt_ctrl = Zero;
        
        {
            const iq20 d_volt_limit = CTRL_VOLT_LIMIT;
            const iq20 d_volt_desired = 
                dq_volt_ff.d 
                + d_curr_err * kp + 
                state.dq_volt_integral.d
            ;
            dq_volt_ctrl.d = math::clamp2(d_volt_desired, d_volt_limit);
            state.dq_volt_integral.d += (dq_volt_ctrl.d - d_volt_desired);
        }
        
        {
            const iq20 q_volt_limit = CTRL_VOLT_LIMIT * math::sat_ucircle(uq32(math::abs(dq_volt_ctrl.d) * INV_CTRL_VOLT_LIMIT));
            const iq20 q_volt_desired = 
                dq_volt_ff.q 
                + q_curr_err * kp 
                + state.dq_volt_integral.q
            ;
            dq_volt_ctrl.q = math::clamp2(q_volt_desired, q_volt_limit);
            state.dq_volt_integral.q += (dq_volt_ctrl.q - q_volt_desired);
        }

        state.dq_volt_ctrl = dq_volt_ctrl;
    }


    const auto inv_busbar_volt = INV_BUSBAR_VOLT;
    const auto inv_busbar_volt_3by2 = (inv_busbar_volt * 3u) >> 1;

    
    {//hfi
        auto alphabeta_dutycycle_final = (state.dq_volt_ctrl * inv_busbar_volt_3by2).rotate(elec_sincos);


        // const bool hfi_enabled = true;
        const auto hfi_method = fn_switches.hfi_method;
        const bool hfi_enabled = hfi_method != HfiMethod::Disabled;
        const bool hfi_use_spin = hfi_method == HfiMethod::Spin;

        if(hfi_enabled){
            auto & table = SINCOS_32STEP_TABLE;
            static constexpr size_t table_size = std::tuple_size_v<std::decay_t<decltype(table)>>;
            static_assert(std::has_single_bit(table_size));

            const auto table_idx_mask = table_size - 1;
            const auto hfi_modu_depth = HFI_MODU_DEPTH_LIMIT;


            auto calc_spin_hfi_dutycycle = [&] -> AlphaBetaCoord<iq20>{
                const auto lg2_len_hfi_samples = 5;
                const auto len_hfi_samples = 1 << lg2_len_hfi_samples;

                const auto table_fact = table_size / len_hfi_samples;
                
                const auto now_hfi_idx = state.hfi_idx;
                if(now_hfi_idx >= len_hfi_samples) __builtin_unreachable();
                const auto [s_bin1,c_bin1] = table[(now_hfi_idx * table_fact) & table_idx_mask];
                const auto [s_bin2,c_bin2] = table[(now_hfi_idx * table_fact * 2) & table_idx_mask];

                const auto hfi_response = dot2v2(
                    state.alphabeta_curr_raw.alpha, c_bin1,
                    state.alphabeta_curr_raw.beta, s_bin1
                );

                if(state.hfi_is_neg_samp){
                    const auto di = hfi_response - state.hfi_response;
                    state.hfi_response = hfi_response;

                    auto next_hfi_idx = now_hfi_idx + 1;


                    if(next_hfi_idx >= len_hfi_samples){
                        next_hfi_idx = 0;

                        state.spinhfi_bin0_real_response = state.spinhfi_bin0_real_response_acc >> lg2_len_hfi_samples;
                        state.spinhfi_bin0_real_response_acc = 0;

                        state.spinhfi_bin2_real_response = state.spinhfi_bin2_real_response_acc >> lg2_len_hfi_samples;
                        state.spinhfi_bin2_real_response_acc = 0;

                        state.spinhfi_bin2_imag_response = state.spinhfi_bin2_imag_response_acc >> lg2_len_hfi_samples;
                        state.spinhfi_bin2_imag_response_acc = 0;

                        state.spinhfi_bin2_real_response_slowlp = lpf_100hz(state.spinhfi_bin2_real_response_slowlp, state.spinhfi_bin2_real_response);
                        state.spinhfi_bin2_imag_response_slowlp = lpf_100hz(state.spinhfi_bin2_imag_response_slowlp, state.spinhfi_bin2_imag_response);
                    }else{
                        state.spinhfi_bin0_real_response_acc += di;

                        state.spinhfi_bin2_real_response_acc += di * c_bin2;
                        state.spinhfi_bin2_imag_response_acc += di * s_bin2;
                    }


                    state.hfi_idx = next_hfi_idx;
                }else{
                    state.hfi_response = hfi_response;
                }


                auto bin2_real_response = state.spinhfi_bin2_real_response;
                auto bin2_imag_response = state.spinhfi_bin2_imag_response;

                if(math::abs(state.hfi_pll_state.angluar_speed_integral.to_turns()) > 10){
                    bin2_real_response -= state.spinhfi_bin2_real_response_slowlp;
                    bin2_imag_response -= state.spinhfi_bin2_imag_response_slowlp;
                }

                HFI_PLL_COEFFS.iterate(state.hfi_pll_state, {
                    iq16((bin2_imag_response) * 35),
                    iq16((bin2_real_response) * 35)
                });

                // HFI_PLL_COEFFS.iterate_err(state.hfi_pll_state,
                //     iq16(iq32(math::atan2pu(bin2_imag_response, bin2_real_response)) - iq32(state.hfi_pll_state.angle.to_turns()))
                // );

                const auto now_hfi_lap_angle2x = state.hfi_pll_state.angle.cast_inner<uq16>();

                const auto hfi_diff_angle2x = (now_hfi_lap_angle2x.cast_inner<iq16>()
                    - state.prev_hfi_lap_angle2x.cast_inner<iq16>()).signed_normalized();

                state.prev_hfi_lap_angle2x = now_hfi_lap_angle2x;
                state.hfi_multilap_angle2x = state.hfi_multilap_angle2x + hfi_diff_angle2x;

                auto hfi_elec_angle = make_angular_from_turns(uq32(math::frac(state.hfi_multilap_angle2x.to_turns() >> 1)));
                state.hfi_elec_angle = hfi_elec_angle;

                if(state.hfi_is_neg_samp){
                    state.hfi_is_neg_samp = false;
                    return AlphaBetaCoord<iq20>{
                        .alpha = hfi_modu_depth * c_bin1,
                        .beta = hfi_modu_depth * s_bin1,
                    };
                }else{
                    state.hfi_is_neg_samp = true;
                    return AlphaBetaCoord<iq20>{
                        .alpha = (-hfi_modu_depth) * c_bin1,
                        .beta = (-hfi_modu_depth) * s_bin1,
                    };
                }
            };




            auto calc_pulse_hfi_dutycycle = [&] -> AlphaBetaCoord<iq20>{
                const auto lg2_len_hfi_samples = 5;
                const auto len_hfi_samples = 1 << lg2_len_hfi_samples;

                const auto table_fact = table_size / len_hfi_samples;

                // const Angular<uq32> est_angle = state.hfi_pll_state.angle;
                #if 0
                const Angular<uq32> est_angle = Zero;
                const auto est_angle_sincos = math::Rotation2<iq31>::from_angle(est_angle);
                #else
                const Angular<uq32> est_angle = state.hfi_pll_state.angle;
                const auto est_angle_sincos = math::Rotation2<iq31>::from_angle(est_angle);

                #endif


                const auto now_hfi_idx = state.hfi_idx;
                if(now_hfi_idx >= len_hfi_samples) __builtin_unreachable();
                const auto [s_bin1,c_bin1] = table[(now_hfi_idx * table_fact) & table_idx_mask];

                const auto est_dq_curr = state.alphabeta_curr_raw.inv_rotate(est_angle_sincos);

                // if(state.hfi_is_neg_samp){
                    // const auto di = hfi_response - state.hfi_response;
                    // state.hfi_response = hfi_response;

                    auto next_hfi_idx = now_hfi_idx + 1;
                    if(next_hfi_idx >= len_hfi_samples){
                        next_hfi_idx = 0;
                    }else{

                    }

                    // state.pulsehfi_q_response = di * c_bin1;
                    state.pulsehfi_q_response = lpf_500hz(state.pulsehfi_q_response, est_dq_curr.q * c_bin1);
                    // state.pulsehfi_q_response = est_dq_curr.q / est_dq_curr.d;
                    // state.pulsehfi_q_response = est_dq_curr.d;


                    state.hfi_idx = next_hfi_idx;

                // }else{
                //     state.hfi_response = hfi_response;
                // }


                HFI_PLL_COEFFS.iterate_err(state.hfi_pll_state, state.pulsehfi_q_response * 0.9_iq16);
                auto hfi_elec_angle = state.hfi_pll_state.angle;
                state.hfi_elec_angle = hfi_elec_angle + make_angular_from_turns(0.5_uq32);


                const auto dq_dutycycle = DqCoord<iq20>{
                    // hfi_modu_depth * (state.hfi_is_neg_samp ? c_bin1 : -c_bin1),
                    hfi_modu_depth * (c_bin1),
                    0.0_iq20
                };

                state.hfi_is_neg_samp = !state.hfi_is_neg_samp;

                return (dq_dutycycle).rotate(est_angle_sincos);

            };


            state.alphabeta_dutycycle_hfi = [&] -> AlphaBetaCoord<iq20> {
                if(hfi_use_spin){
                    return calc_spin_hfi_dutycycle();
                }else{
                    return calc_pulse_hfi_dutycycle();
                }
                __builtin_unreachable();
            }();

            alphabeta_dutycycle_final = alphabeta_dutycycle_final + state.alphabeta_dutycycle_hfi;
        }

        state.alphabeta_dutycycle_final = alphabeta_dutycycle_final;
        state.alphabeta_volt_final = alphabeta_dutycycle_final * BUSBAR_VOLT;
    }


    {
        auto uvw_dutycycle_genout = SVM(state.alphabeta_dutycycle_final);

        // const bool deadtime_comp_en = true;
        const bool deadtime_comp_en = fn_switches.deadtime_compensate_en;

        if(deadtime_comp_en){
            // https://www.zhihu.com/question/270446098/answer/3215795384
            // 《SVPWM逆变器死区补偿的研究与实现》 魏凯

            const auto uvw_curr_fastlp = state.uvw_curr_fastlp;
            [[maybe_unused]] const auto weak_flip_threshold = CURRENT_AMPS_PER_ADC_LSB * 3;
            [[maybe_unused]] const auto strong_flip_threshold = CURRENT_AMPS_PER_ADC_LSB * 6;
            static constexpr auto DEADTIME_COMP_DUTYCYCLE = iq16(
                (DEADTIME_NANOS.count() * FOC_FREQ * 1e-9)
            );

            // [[maybe_unused]] static constexpr auto f = (float)DEADTIME_COMP_DUTYCYCLE;

            [[maybe_unused]] static constexpr auto ONE_BY_3 = uq32(1.0 / 3.0);
            [[maybe_unused]] static constexpr auto TWO_BY_3 = uq32(2.0 / 3.0);
            [[maybe_unused]] static constexpr auto ONE_BY_SQRT3 = uq32(1.0 / 1.73205080757);


            UvwCoord<iq16> uvw_dutycycle_deadcomp;
            [[maybe_unused]] auto & state_sign = state.deadcomp_state.uvw_sign;
            [[maybe_unused]] auto & state_strong = state.deadcomp_state.uvw_strong;
            [[maybe_unused]] auto prev_sign = state.deadcomp_state.uvw_sign;
            [[maybe_unused]] auto prev_strong = state.deadcomp_state.uvw_strong;
            
            #if 0

            #define REDETECT_PHASE(idx)\
            if(prev_strong[idx]){\
                if(prev_sign[idx] >= 0){\
                    if(uvw_curr_fastlp[idx] < strong_flip_threshold){\
                        state_sign[idx] = -1;\
                        state_strong[idx] = false;\
                    }\
                }else{\
                    if(uvw_curr_fastlp[idx] > -strong_flip_threshold){\
                        state_sign[idx] = 1;\
                        state_strong[idx] = false;\
                    }\
                }\
            }else{\
                if(prev_sign[idx] >= 0){\
                    if(uvw_curr_fastlp[idx] < -weak_flip_threshold){\
                        state_sign[idx] = -1;\
                        state_strong[idx] = true;\
                    }\
                }else{\
                    if(uvw_curr_fastlp[idx] > weak_flip_threshold){\
                        state_sign[idx] = 1;\
                        state_strong[idx] = true;\
                    }\
                }\
            }
            #else

            #define REDETECT_PHASE(idx)\
            state_sign[idx] = uvw_curr_fastlp[idx] > 0 ? 1 : -1;

            #endif


            REDETECT_PHASE(0)
            REDETECT_PHASE(1)
            REDETECT_PHASE(2)
            // uvw_dutycycle_deadcomp[0] = (prev_sign[1] + prev_sign[2]) * DEADTIME_COMP_DUTYCYCLE;
            // uvw_dutycycle_deadcomp[1] = (prev_sign[2] + prev_sign[0]) * DEADTIME_COMP_DUTYCYCLE;
            // uvw_dutycycle_deadcomp[2] = (prev_sign[0] + prev_sign[1]) * DEADTIME_COMP_DUTYCYCLE;

            uvw_dutycycle_deadcomp[0] = (prev_sign[0]) * (DEADTIME_COMP_DUTYCYCLE * TWO_BY_3);
            uvw_dutycycle_deadcomp[1] = (prev_sign[1]) * (DEADTIME_COMP_DUTYCYCLE * TWO_BY_3);
            uvw_dutycycle_deadcomp[2] = (prev_sign[2]) * (DEADTIME_COMP_DUTYCYCLE * TWO_BY_3);

            uvw_dutycycle_genout = uvw_dutycycle_genout + uvw_dutycycle_deadcomp;
            state.uvw_dutycycle_deadcomp = uvw_dutycycle_deadcomp;
        }

        state.uvw_dutycycle_genout = uvw_dutycycle_genout;

    }
};

void myesc_main(){
    //等待板载外围原件稳定（如SPI编码器）
    clock::delay(12ms);

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
        .set_eps(5)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();
    // DEBUGGER.force_sync(EN);
    // while(true){
    //     const auto now_secs = clock::seconds();
    //     const auto frac_secs = math::frac(now_secs);
    //     // DEBUG_PRINTLN(clock::millis());
    //     // DEBUG_PRINTLN(sat_left2(uq32(now_secs)));
    //     DEBUG_PRINTLN(mysat(frac_secs), mysat(uq32(frac_secs)));
    // }


    clock::delay(2ms);


    // #region 初始化ADC


    setup_adc();

    // #endregion


    // #region 初始化DRV8323
    setup_drv8323();


    // #endregion


    // #region 初始化定时器

    setup_timer();
    hal::PA<6>().inflt();

    stop_pwm();

    //确保pwm完全停止
    clock::delay(2ms);

    // #endregion 初始化定时器

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

    #if 1
    auto mag_encoder_ = VCE2755{
        &spi,
        spi.allocate_cs_pin(&mag_encoder_cs_pin_)
            .unwrap()
    };

    mag_encoder_.init(Default).examine();
    mag_encoder_.set_direction(CW).examine();
    mag_encoder_.set_filter_bandwidth(VCE2755::FilterBandwidth::_8BW0).examine();

    auto mag_encoder_get_angle = [&] -> Angular<uq32>{
        return mag_encoder_.get_angle().examine().parse().unwrap();
    };

    for(size_t i = 0; i < 100; i++){
        (void)mag_encoder_get_angle();
        clock::delay(100us);
    }
    // mag_encoder_.set_filter_bandwidth(VCE2755::FilterBandwidth::_BW0).examine();
    #endif


    // #endregion


    // #region 初始化LED
    auto led_blue_pin_ = hal::PC<13>();
    auto led_red_pin_ = hal::PC<14>();
    auto led_green_pin_ = hal::PC<15>();

    led_red_pin_.outpp();
    led_blue_pin_.outpp();
    led_green_pin_.outpp();

    [[maybe_unused]] auto poll_led_blink = [&]{

        led_red_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        led_blue_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
        led_green_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 800) > 400);
    };


    // #endregion


    auto op_flags_ = OpFlags::from_default();
    op_flags_.dc_calibrate_unready = true;
    op_flags_.sideshaft_calibrate_unready = false;

    auto fn_switches_ = FnSwitches::from_default();
    fn_switches_.current_harmonic_suppression_en = 0;
    fn_switches_.elec_angle_source = ElecAngleSource::MagEncoder;
    fn_switches_.loop_wiring = LoopWiring::SeriesPi;
    // fn_switches_.traj_smooth_method = TrajSmoothMethod::UseX1AndZero;
    fn_switches_.traj_smooth_method = TrajSmoothMethod::UseX1AndX2;
    // fn_switches_.traj_smooth_method = TrajSmoothMethod::Disabled;

    auto p_all_state_ = std::make_unique<AllState>();
    AllState & all_state_ = *p_all_state_;
    all_state_.reset();
    all_state_.torque_curr_cmd = 0.0_iq20;



    auto jeoc_isr = [&]{

        static constexpr auto POLE_PAIRS = MotorProfile::POLE_PAIRS;
        static constexpr size_t ENCODER_SIDESHAFT_EPS_TABLE_LENGTH = POLE_PAIRS * 6;
        static_assert(ENCODER_SIDESHAFT_EPS_TABLE_LENGTH < ENCODER_SIDESHAFT_EPS_TABLE_CAPACITY);
        static constexpr size_t SIDESHAFT_CALIBRATE_OVERSAMPLES = 16;
        static constexpr size_t STEPS_PER_CELL_PER_REV = 256 * 2;
        static constexpr size_t SAMPLE_DURATION_STEPS = STEPS_PER_CELL_PER_REV / SIDESHAFT_CALIBRATE_OVERSAMPLES;
        static constexpr size_t REVS_PER_DIRECTION = 2;
        static constexpr size_t STEPS_PER_REV = (STEPS_PER_CELL_PER_REV * ENCODER_SIDESHAFT_EPS_TABLE_LENGTH);
        
        // static constexpr bool do_sideshaft_calibrate = false;
        static constexpr bool sideshaft_compensate = true;
        // static constexpr bool sideshaft_compensate = fn_switches.sideshaft_compenstate_en;
        auto  & state = all_state_;

        //do clone, avoid external modify during isr
        const volatile uint32_t fn_switches_bits = std::bit_cast<uint32_t>(fn_switches_);
        const auto fn_switches = std::bit_cast<FnSwitches>(uint32_t(fn_switches_bits));

        // const volatile uint32_t op_flags_bits = std::bit_cast<uint32_t>(op_flags_);
        // const auto op_flags = std::bit_cast<FnSwitches>(uint32_t(op_flags_bits));
        auto & op_flags = op_flags_;

        timming_watch_pin_.set_high();


        state.isr_entry_tick = get_timer_tick();


        const bool sideshaft_calibrate_done = !op_flags.sideshaft_calibrate_unready;
        const bool do_sideshaft_calibrate = sideshaft_compensate and (!sideshaft_calibrate_done);

        #if 1
        // if(true){
        // if(false){
        const auto encoder_mech_angle_uncompensated = mag_encoder_get_angle();

        auto warp_encoder_err = [](const uq32 ref, const uq32 meas) -> iq32{
            return ((iq32(ref) - iq32(meas)) * POLE_PAIRS) * uq32(1.0 / POLE_PAIRS);
        };

        auto steps_to_turns = [](const size_t steps) -> uq32{
            const size_t rem = steps % STEPS_PER_REV;
            constexpr uq32 FACTOR = uq32::from_bits(uint32_t(float(1ull << 32) / STEPS_PER_REV) + 1);
            return rem * FACTOR;
        };



        auto calc_encoder_eps = [](const iq32 * table_data, const uq32 raw_turns) -> iq32{
            auto index = intrinsics::mul32hu(ENCODER_SIDESHAFT_EPS_TABLE_LENGTH, raw_turns.to_bits());
            const auto frac = (raw_turns * ENCODER_SIDESHAFT_EPS_TABLE_LENGTH);

            auto next_index = warp_index(index + 1, ENCODER_SIDESHAFT_EPS_TABLE_LENGTH);

            static constexpr size_t RIGHT_SHIFTS = pow2(SIDESHAFT_CALIBRATE_OVERSAMPLES * REVS_PER_DIRECTION);
            const auto now_cell_value = table_data[index];
            const auto next_cell_value = table_data[next_index];

            const auto table_eps = lerp_fixed_uq32(now_cell_value, next_cell_value, frac) >> RIGHT_SHIFTS;
            return table_eps;
        };

        const auto encoder_mech_angle = [&](const iq32 * table_data, uq32 raw_turns) -> Angular<uq32>{
            if(sideshaft_calibrate_done & fn_switches.sideshaft_compenstate_en){
            // if(false){
                const auto table_eps = calc_encoder_eps(table_data, raw_turns);
                raw_turns = raw_turns + table_eps;
            }
            return make_angular_from_turns<uq32>(raw_turns);
        }(state.encoder_calibrate_state.eps_table.data(), encoder_mech_angle_uncompensated.to_turns());

        state.encoder_abs_turns64 = uq32_wrapped_update(
            state.encoder_abs_turns64, encoder_mech_angle.to_turns());
        #endif

        state.encoder_get_done_tick = get_timer_tick();


        const auto uvw_bvalue = get_adc_uvw_bvalue();
        process_current_sense(state, fn_switches, uvw_bvalue);


        if(op_flags.dc_calibrate_unready){
            auto & dc_state = state.dc_calibrate_state;
            dc_state.uvw_bvalue_offset_acc = {
                std::get<0>(dc_state.uvw_bvalue_offset_acc) + int32_t(std::get<0>(uvw_bvalue)),
                std::get<1>(dc_state.uvw_bvalue_offset_acc) + int32_t(std::get<1>(uvw_bvalue)),
                std::get<2>(dc_state.uvw_bvalue_offset_acc) + int32_t(std::get<2>(uvw_bvalue))
            };
            dc_state.dc_cal_cnt++;
            if(dc_state.dc_cal_cnt >= DC_CAL_TIMES){
                dc_state.uvw_bvalue_offset = {
                    int32_t(std::get<0>(dc_state.uvw_bvalue_offset_acc) >> LG2_DC_CAL_TIMES),
                    int32_t(std::get<1>(dc_state.uvw_bvalue_offset_acc) >> LG2_DC_CAL_TIMES),
                    int32_t(std::get<2>(dc_state.uvw_bvalue_offset_acc) >> LG2_DC_CAL_TIMES)
                };
                op_flags.dc_calibrate_unready = false;
            }

            {
                const auto LOW_HFI_DUTYCYCLE = iq20(0.05_iq16);
                const auto [s,c] = dsp::SINCOS_32STEP_TABLE[dc_state.dc_cal_cnt & 0x1f];
                const auto alphabeta_dutycycle = AlphaBetaCoord<iq16>{
                    .alpha = c * LOW_HFI_DUTYCYCLE,
                    .beta = s * LOW_HFI_DUTYCYCLE
                };

                state.uvw_dutycycle_genout = (SVM(alphabeta_dutycycle * iq16(1.5)));
            }

        }else{
            process_position_sense(state, fn_switches);

            if(not do_sideshaft_calibrate){
                process_traj_generate(state, fn_switches);
                process_traj_shape(state, fn_switches);
                process_mechanical_loop(state, fn_switches);
            }


            if(do_sideshaft_calibrate){
                {

                    static constexpr size_t LG2_STEPS_CURRENT_RAMP = 14u;

                    static constexpr iq16 ELEC_X2 = iq16(FOC_FREQ * 1.0 / STEPS_PER_REV);
                    [[maybe_unused]] static constexpr float SECONDS_PER_REV = double(STEPS_PER_REV) / FOC_FREQ;
                    [[maybe_unused]] static constexpr float SECONDS_RAMP = double(1 << LG2_STEPS_CURRENT_RAMP) / FOC_FREQ;

                    using Counter = EncoderNonlinearCalibrateCounter;
                    using Stage = EncoderNonlinearCalibrateStage;
                    
                    constexpr auto override_fn_switches = SIDESHAFT_CALIBRATE_SWITCHES;
                    constexpr iq20 calibrate_curr_limit = iq20(2.0); 
                    constexpr iq20 torque_curr_delta = calibrate_curr_limit >> LG2_STEPS_CURRENT_RAMP;
                    
                    auto & ec_state = state.encoder_calibrate_state;


                    const auto now_counter = Counter{ec_state.counter};
                    const auto now_stage = now_counter.stage().get();
                    // const auto now_specifier = now_counter.specifier().get();
                    const auto now_count_value = now_counter.count_value().get();

                    Stage next_stage = Stage::Failed;
                    // uint8_t next_specifier = now_specifier;
                    uint32_t next_count_value = 0;

                    auto _g = make_scope_guard([&]{
                        Counter counter{0};
                        counter.stage().set(next_stage);
                        // counter.specifier().set(next_specifier);
                        counter.count_value().set(next_count_value);
                        ec_state.counter = counter.bits;
                    });



                    const auto encoder_mech_turns = encoder_mech_angle_uncompensated.to_turns();

                    switch(now_stage){
                        case Stage::Start:{
                            next_stage = Stage::Ramp;
                            // next_specifier = uint8_t(1);
                            next_count_value = 0;
                            break;
                        }

                        case Stage::Ramp:{
                            next_stage = Stage::Ramp;
                            next_count_value = now_count_value + 1;

                            if(next_count_value >= (1u << LG2_STEPS_CURRENT_RAMP)){
                                next_stage = Stage::Forward;
                                next_count_value = 0;
                            }

                            const auto torque_curr = torque_curr_delta * now_count_value;

                            ec_state.cmd_mech_turns = 0;
                            ec_state.torque_curr = torque_curr;
                            break;
                        }

                        case Stage::Forward:{
                            next_stage = Stage::Forward;
                            next_count_value = now_count_value + 1;
                            if(next_count_value >= STEPS_PER_REV * REVS_PER_DIRECTION){
                                next_stage = Stage::Backward;
                                next_count_value = 0;
                            }

                            const auto now_steps = now_count_value;

                            const auto cmd_mech_turns = ec_state.cmd_mech_turns = steps_to_turns(now_steps);
                            
                            const auto mech_eps = warp_encoder_err(cmd_mech_turns, encoder_mech_turns);

                            const auto frac = (encoder_mech_turns * ENCODER_SIDESHAFT_EPS_TABLE_LENGTH);
                            const auto index = intrinsics::mul32hu(ENCODER_SIDESHAFT_EPS_TABLE_LENGTH, encoder_mech_turns.to_bits());

                            const auto table_index = warp_index(index + (frac >= 0.5_uq32), ENCODER_SIDESHAFT_EPS_TABLE_LENGTH);

                            if(now_count_value % (SAMPLE_DURATION_STEPS) == 0){
                                ec_state.eps_table[table_index] += mech_eps;
                            }

                            ec_state.torque_curr = calibrate_curr_limit;
                            ec_state.debug.mech_eps_before = mech_eps;
                            ec_state.debug.index = table_index;
                            break;
                        }
                        
                        case Stage::Backward:{
                            next_stage = Stage::Backward;
                            next_count_value = now_count_value + 1;

                            if(next_count_value >= STEPS_PER_REV * REVS_PER_DIRECTION){
                            // if(next_count_value >= 0){
                                next_stage = Stage::Complete;
                                next_count_value = 0;
                            }

                            const auto now_steps = now_count_value;

                            const auto cmd_mech_turns = ec_state.cmd_mech_turns = steps_to_turns(now_steps);
                            
                            const auto table_eps = calc_encoder_eps(ec_state.eps_table.data(), encoder_mech_turns);
                            [[maybe_unused]] const auto enc_mech_turns_after = encoder_mech_turns + table_eps;

                            ec_state.debug.mech_eps_before = warp_encoder_err(cmd_mech_turns, encoder_mech_turns);
                            ec_state.debug.mech_eps_after = warp_encoder_err(cmd_mech_turns, enc_mech_turns_after);

                            ec_state.torque_curr = calibrate_curr_limit;

                            break;
                        }
                        case Stage::Complete:{
                            op_flags.sideshaft_calibrate_unready = false;
                            break;
                        }
                        case Stage::Failed:{
                            break;
                        }
                    }
                    process_current_loop(state, override_fn_switches, 
                        // Angular<uq32>::from_turns(cmd_mech_turns * POLE_PAIRS + 0.25_uq32), ELEC_X2, 1.0_iq20);
                        Angular<uq32>::from_turns(ec_state.cmd_mech_turns * POLE_PAIRS - 0.5_uq32), ELEC_X2, ec_state.torque_curr);
                }
            }else{

                process_current_loop(state, fn_switches, 
                    state.elec_angle, state.elec_speed, state.torque_curr_cmd);
            }


        }

        {
            set_uvw_dutycycle(state.uvw_dutycycle_genout, fn_switches.phase_invert_en);
        }

        state.isr_exit_tick = get_timer_tick();
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

    start_pwm();

    clock::delay(2ms);

    const auto TEMP_TRIMER = hal::TemperatureTrimer::load();



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

            
            script::make_function(StringView("esrc"), [&](const uint8_t s){
                fn_switches_.elec_angle_source = ElecAngleSource(s);
            }),

            script::make_function(StringView("lpw"), [&](const uint8_t s){
                fn_switches_.loop_wiring = LoopWiring(s);
            }),

            script::make_function(StringView("tj1"), [&](const iq16 x){
                all_state_.traj_state.x1 = x;
            }),
            
            script::make_function(StringView("sce"), [&](const bool en){
                fn_switches_.sideshaft_compenstate_en = en;
            }),

            // script::make_function(StringView("hse"), [&](const bool en){
            //     fn_switches_.current_harmonic_suppression_en = en;
            // }),

            // script::make_function(StringView("ivt"), [&](const bool en){
            //     fn_switches_.phase_invert_en = en;
            // }),
            
            script::make_function(StringView("rst"), [&](){
                sys::reset();
            })
        );

        repl_server.invoke(list);
    };
    


    while(true){
        auto & state = all_state_;

        poll_repl_activity();

        // const auto angle = mag_encoder_.update().examine().parse().unwrap();
        [[maybe_unused]] const auto now_secs = clock::seconds();

        state.temperature_state.die().celsius = lpf_10hz(state.temperature_state.die().celsius, TEMP_TRIMER.parse_u12(ADC1->IDATAR4));

        // {
            const auto offset = state.spinhfi_bin0_real_response;
            const auto amp = math::mag(state.spinhfi_bin2_real_response, state.spinhfi_bin2_imag_response) * 2;
            static constexpr auto factor = (int)(iq12(FOC_FREQ) / iq12(HFI_MODU_DEPTH_LIMIT * BUSBAR_VOLT));
            [[maybe_unused]] const auto lq_est_mh = iq20(1000.0 / factor) / (offset - amp);
            [[maybe_unused]] const auto ld_est_mh = iq20(1000.0 / factor) / (offset + amp);
        // }

        // {
            [[maybe_unused]] const auto mag_volt = math::mag(state.dq_volt_integral[0], state.dq_volt_integral[1]);
            [[maybe_unused]] const auto inv_mag_curr = math::inv_mag(state.alphabeta_curr_raw[0], state.alphabeta_curr_raw[1]);
        // }

        if(false)DEBUG_PRINTLN();
            // die_celsius_,
            // s, c, 
            // pwm_u_.cvr(),
            // pwm_v_.cvr(),
            // pwm_w_.cvr(),
            // state.uvw_curr_raw.u,
            // state.uvw_curr_raw.v + state.uvw_curr_raw.w,
            // state.uvw_curr_raw.v,
            // state.uvw_curr_raw.w,
            // state.uvw_curr_slowlp.u,
            // state.uvw_curr_slowlp.v + state.uvw_curr_slowlp.w,
            // u_disconn_dbs.count,
            // v_disconn_dbs.count,
            // judge_is_disconn(state.uvw_curr_slowlp.u,state.uvw_curr_slowlp.v + state.uvw_curr_slowlp.w),
            // judge_is_disconn(state.uvw_curr_slowlp.v,state.uvw_curr_slowlp.u + state.uvw_curr_slowlp.w),
            // state.alphabeta_volt_final.alpha,
            // state.alphabeta_volt_final.beta,
            // state.d_volt_gen,
            // state.q_volt_gen,
            // flux_sensorless_ob.angle().to_turns(),
            // flux_sensorless_ob.state().flux_state_mf[0],
            // flux_sensorless_ob.state().flux_state_mf[1],
            // flux_sensorless_ob.state().v_alphabeta_last[0],
            // flux_sensorless_ob.state().v_alphabeta_last[1],
            // state.torque_curr_cmd,
            // state.uvw_curr_raw,
            // state.dq_volt_ctrl,
            // state.elec_angle.to_turns(),
            // state.alphabeta_curr_raw.alpha,
            // state.alphabeta_curr_raw.beta,
            // state.alphabeta_dutycycle_hfi,
            // state.hfi_idx,
            // state.hfi_is_neg_samp,
            // state.dq_volt_ctrl.q,
            // state.dq_curr_raw.q,

            // state.spinhfi_bin2_real_response, 
            // state.spinhfi_bin0_real_response,
            // state.spinhfi_bin2_imag_response, 
            // state.spinhfi_bin2_imag_response + state.spinhfi_bin0_real_response, 
            // state.spinhfi_bin2_imag_response, 
            // state.spinhfi_bin1_imag_response
            // ),
            // state.spinhfi_bin2_real_response_slowlp,
            // state.spinhfi_bin2_imag_response_slowlp,
            // state.alphabeta_curr_raw.alpha,
            // state.dq_volt_ctrl.d,
            // state.dq_volt_ctrl.q,
            // mag_volt,
            // inv_mag_curr,
            // state.uvw_curr_raw,
            
            // lq_est_mh,
            // ld_est_mh,

            
            // state.openloop_elec_angle.to_turns(),
            // state.hfi_elec_angle.to_turns(),
            // state.uvw_dutycycle_genout,
            // state.uvw_dutycycle_deadcomp,
            // state.deadcomp_state.uvw_sign,
            // state.pulsehfi_q_response,
            // state.alphabeta_curr_raw.length(),
            // state.dq_volt_ctrl.length(),
            // state.dq_volt_ctrl.length() / state.alphabeta_curr_raw.length(),
            // state.alphabeta_volt_final.length(),
            // math::atan2(state.spinhfi_bin1_imag_response,
            //     state.spinhfi_bin1_real_response),

            // math::atan2(state.spinhfi_bin2_imag_response,
            //     state.spinhfi_bin2_real_response) / 2,
            // state.openloop_elec_angle.to_turns(),
            // state.hfi_pll_state.angle.to_turns(),
            // state.hfi_elec_angle.to_turns(),
            // state.openloop_elec_angle.to_turns(),
            // state.hfi_elec_angle.to_turns(),
            // state.hfi_pll_state.angle.to_turns(),
            // state.hfi_pll_state.angle.to_turns(),
            // state.hfi_pll_state.angluar_speed.to_turns(),
            // state.hfi_elec_angle.to_turns(),
            // state.temperature_state.die().to_celsius(),

            // state.hybrid_elec_angle.to_turns(),
            // state.observer_hybrid_ratio,

            // offset / (offset * offset - amp * amp),

            // iq12(offset) * int(1000000/ factor),
            // iq12(amp) * int(1000000/ factor),
            // state.hfi_elec_angle.to_turns(),
            // spinhfi_bin2_angle.to_turns(),

            // hfi_idx,
            // state.alphabeta_curr_raw[0],
            // state.alphabeta_curr_raw[1],
            // state.deadtime_comp_alphabeta_dutycycle,
            // state.uvw_dutycycle_genout,

            // state.spinhfi_bin2_imag_response,
            // state.spinhfi_bin2_real_response
            // full_arr,
            // state.isr_elapsed_ticks.count(),
            
            // die_celsius_,
            // flux_sensorless_ob.angle().to_turns(),
            // math::atan2pu(state.alphabeta_curr_raw[0], state.alphabeta_curr_raw[1]),
            // state.alphabeta_curr_raw[0] / state.alphabeta_curr_raw[1],
            // state.uvw_curr_ref,
            // state.alphabeta_curr_raw,
            // state.alphabeta_curr_ref,
            // state.alphabeta_curr_raw,
            // state.dq5_curr_lp,
            // state.harmonic_state.id6c,
            // state.harmonic_state.id6s,
            // state.harmonic_state.iq6c,
            // state.harmonic_state.iq6s,
            // state.dq5_curr_lp[0],
            // state.dq5_curr_lp[1],
            // state.dq7_curr_lp[0],
            // state.dq7_curr_lp[1],
            // state.harmonic_state.vs5c_integral,
            // state.harmonic_state.vs5s_integral,
            // state.harmonic_state.vs7c_integral,
            // state.harmonic_state.vs7s_integral,
            // state.harmonic_state.vd6c_integral,
            // state.harmonic_state.vd6s_integral,
            // state.harmonic_state.vq6c_integral,
            // state.harmonic_state.vq6s_integral,

            // state.harmonic_state.delta_vd6_in,
            // state.harmonic_state.delta_vq6_in,
            // tmrticks_to_us(state.isr_entry_tick),
            // tmrticks_to_us(state.isr_exit_tick),
            // state.busbar_curr_lp,
            // state.dq_curr_raw.q,
            // state.dq_curr_raw.d,

            // state.flux_ob_state.lem1,
            // state.flux_ob_state.lem2,
            // state.uvw_dutycycle_genout,
            // state.dq_volt_ctrl.q,
            // state.dq_volt_ctrl.d,
            // state.dq_curr_raw.d,
            // state.dq_curr_raw.q,
            // state.busbar_curr_lp,
            // state.uvw_curr_raw.w,
            // math::atan2pu(state.flux_ob_state.x2,

            // state.observer_pll_state.angluar_speed.to_turns(),
            // state.flux_ob_state.lem1,
            // state.flux_ob_state.lem2,
            // state.flux_ob_state.flux_err,
            // state.flux_ob_state.abs_lem,
            // state.flux_ob_state.x2,
            // state.flux_ob_state.x2_slowlp,
            // state.sensed_elec_angle.to_turns(),
            // state.busbar_curr_lp,
            // state.torque_curr_cmd,
            // state.dq_curr_raw,
            // state.dq_curr_ref,
            // state.speed_eso_state.speed_est,
            // state.to_turns(),
            // state.sensed_elec_speed,

            // uint16_t(TIM_INST->ATRLR)
            // timer.oc<4>().cvr(),
            // timer.arr()
            // state.isr_elapsed_ticks.count()
            // state.alphabeta_volt_final,
            // state.alphabeta_curr_raw
            // state.uvw_curr_raw.u,
            // state.uvw_curr_raw.v,
            // state.uvw_curr_raw.w

        if(false)DEBUG_PRINTLN(
            state.traj_state.x1,
            math::fixed_downcast<16>(state.curve_state.x1),
            math::fixed_downcast<16>(state.encoder_state_2o.x1),
            // iq16::from_bits(int32_t(differential_int64(state.differ, state.curve_state.x1.to_bits()) >> 6)),
            math::fixed_downcast<16>(state.curve_state.x1) - math::fixed_downcast<16>(state.encoder_state_2o.x1),
            state.encoder_state_2o.x2,
            // mag_volt * inv_mag_curr,
            // state.busbar_curr_lp * BUSBAR_VOLT,
            // state.flux_ob_state.x1,
            // state.flux_ob_state.x2,
            // state.torque_curr_cmd,
            // state.curve_state.x3,
            // state.curve_state.u,
            tmrticks_to_us(state.isr_exit_tick) - tmrticks_to_us(state.isr_entry_tick)
            // tmrticks_to_us(state.encoder_get_done_tick) - tmrticks_to_us(state.isr_entry_tick)
        );

        // auto & ec_state = state.encoder_calibrate_state;
        if(true)DEBUG_PRINTLN(
            state.traj_state.x1,
            math::fixed_downcast<16>(state.curve_state.x1),
            math::fixed_downcast<16>(state.encoder_state_2o.x1),
            // uq32::from_bits((state.encoder_abs_turns64.to_bits() & UINT32_MAX) * MotorProfile::POLE_PAIRS * 6),
            // math::fixed_downcast<16>(state.encoder_state_2o.x1 * MotorProfile::POLE_PAIRS),
            // iq16::from_bits(int32_t(differential_int64(state.differ, state.curve_state.x1.to_bits()) >> 6)),
            state.encoder_state_2o.x2,
            (state.curve_state.x2),
            // uq32::from_bits(state.encoder_state_2o.x1.to_bits() & UINT32_MAX),
            // ec_state.cmd_mech_turns,
            // ec_state.debug.mech_eps_before,
            // ec_state.debug.mech_eps_after,
            // ec_state.torque_curr,
            // ec_state.debug.index,
            // (uint8_t)EncoderNonlinearCalibrateCounter{ec_state.counter}.stage().get(),
            // EncoderNonlinearCalibrateCounter{ec_state.counter}.count_value().get(),
            state.torque_curr_cmd,
            state.pi_x2_ref,
            state.pi_e2,
            state.torque_curr_integral,
            tmrticks_to_us(state.isr_exit_tick) - tmrticks_to_us(state.isr_entry_tick)
        );

        poll_led_blink();
        // toggle_red_led();
        // repl_service_poller();
    }

}

