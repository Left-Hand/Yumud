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


#include "drivers/encoder/magnetic/MT6825/mt6825.hpp"
#include "drivers/encoder/magnetic/VCE2755/vce2755.hpp"
#include "drivers/gatedrv/DRV832X/DRV8323h.hpp"

#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"
#include "dsp/controller/adrc/nonlinear/nltd2o.hpp"
#include "dsp/controller/adrc/linear/ltd2o.hpp"

#include "motor_dsp/dsp_lpf.hpp"
#include "motor_dsp/dsp_vec.hpp"
#include "motor_dsp/dsp_fft32.hpp"
#include "motor_dsp/dsp_pi.hpp"
#include "motor_config.hpp"

#include "core/math/clamp.hpp"
#include "core/sdk.hpp"

#include "core/string/owned/thrifty_string.hpp"
#include "core/intrinsics/memop.h"
#include "core/mem/arena.hpp"
#include "roundtrip_traj_generator.hpp"
#include "hal/analog/adc/adc_lld.hpp"

#include "core/string/conv/fmtnum/fmtnum.hpp"

using namespace ymd;
using namespace ymd::myesc;


#define DBG_UART hal::usart2

// template<size_t Q>
// static constexpr traingle_pu(const uq32 t, const )




template<size_t Q, typename D>
requires(sizeof(D) == 4)
static constexpr iiq32 make_iiq32(const math::fixed<Q, D> x){
    const D bits = x.to_bits();
    constexpr size_t LEFT_SHIFTS = 32 - Q;
    return iiq32::from_bits(int64_t(bits) << LEFT_SHIFTS);
} 

static constexpr uq32 iiq32_crop_frac(const iiq32 x){
    return uq32::from_bits(uint32_t(x.to_bits() & UINT32_MAX));
}

static constexpr int32_t iiq32_crop_revs(const iiq32 x){
    return int32_t(x.to_bits() >> 32);
}


static constexpr iiq32 iiq32_add_revs(const iiq32 x, const int32_t n_revs){
    const auto frac = iiq32_crop_frac(x);
    const int32_t revs = iiq32_crop_revs(x);
    return iiq32::from_bits(int64_t(int64_t(revs + n_revs) << 32) | frac.to_bits());
}

// static_assert(iiq32_add_revs(iiq32(0.2f), 1).to_bits() == iiq32(1.2f).to_bits());

template<size_t Q_final, typename D_final, size_t Q, typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q_final, D_final> 
sub_clamp2_downcast(math::fixed<Q, D> lhs, math::fixed<Q, D> rhs, math::fixed<Q_final, D_final> ma){
    constexpr size_t RIGHT_SHIFTS = Q - Q_final;
    D y_bits = D((lhs.to_bits() - rhs.to_bits()));
    const D ma_bits = D(ma.to_bits()) << RIGHT_SHIFTS;
    if(y_bits > ma_bits) y_bits = ma_bits;
    if(y_bits < -ma_bits) y_bits = -ma_bits;
    return math::fixed<Q_final, D_final>::from_bits(y_bits >> RIGHT_SHIFTS);
}

static_assert(sub_clamp2_downcast(iiq32(1000), iiq32(-200), iq20(1000)).to_bits() == iq20(1000).to_bits());
static_assert(sub_clamp2_downcast(iiq32(6000), iiq32(-800), iq20(1000)).to_bits() == iq20(1000).to_bits());
static_assert(sub_clamp2_downcast(iiq32(-6000), iiq32(-800), iq20(1000)).to_bits() == iq20(-1000).to_bits());
static_assert(sub_clamp2_downcast(iiq32(0), iiq32(-800), iq20(1000)).to_bits() == iq20(800).to_bits());



__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<20, int32_t> 
sub_clamp2_downcast_iq20(iiq32 lhs, iiq32 rhs, int32_t ma){
    #if 1
    constexpr size_t RIGHT_SHIFTS = 32 - 20;

    int64_t y_bits = int64_t((lhs.to_bits() - rhs.to_bits()));
    const int32_t head_bits = int32_t(y_bits >> 32);

    if(head_bits > ma) return iq20(ma);
    if(head_bits < -ma) return iq20(-ma);
    return iq20::from_bits(int32_t(y_bits >> RIGHT_SHIFTS));
    #else
    return sub_clamp2_downcast<20, int32_t>(lhs, rhs, iq20(ma));
    #endif
}

static_assert(sub_clamp2_downcast_iq20(iiq32(1000), iiq32(-200), (1000)).to_bits() == iq20(1000).to_bits());
static_assert(sub_clamp2_downcast_iq20(iiq32(6000), iiq32(-800), (1000)).to_bits() == iq20(1000).to_bits());
static_assert(sub_clamp2_downcast_iq20(iiq32(-6000), iiq32(-800), (1000)).to_bits() == iq20(-1000).to_bits());
static_assert(sub_clamp2_downcast_iq20(iiq32(3.5), iiq32(1.2), (1000)).to_bits() == iq20(2.3).to_bits());
// static_assert(sub_clamp2_downcast_iq20(iiq32(13.5), iiq32(1.2), (1000)).to_bits() == iq20(12.3).to_bits());
static_assert(sub_clamp2_downcast_iq20(iiq32(0), iiq32(-800), (1000)).to_bits() == iq20(800).to_bits());



enum class [[nodiscard]] DemoTrajPattern:uint8_t{
    Stop,
    Straight,
    Sine,
    Saw,
    Stairs,
    Triangle,
    Miniwave
};


__no_inline static constexpr TrajState
calc_demo_traj(const uq16 t, const DemoTrajPattern demo_pattern){
    switch(demo_pattern){
        case DemoTrajPattern::Stop:{
            return {0, 0, 0};
        }
        case DemoTrajPattern::Straight:{
            constexpr auto speed = 0.03_iq16;

            return {
                make_iiq32(speed * t),
                iq20(speed),
                0
            };
        }

        case DemoTrajPattern::Triangle:{
            static constexpr auto abs_delta = iq32(1.0 / 65536 / 16);
            static iiq32 position = 0;
            static constexpr int flip_duration = 32;
            const bool is_forward = int(t) % (flip_duration * 2) < flip_duration;

            constexpr iq32 abs_speed = iq32(abs_delta * CONF_FOC_FREQ);
            [[maybe_unused]] constexpr float abs_speed_f = (float)abs_speed;

            auto speed = abs_speed;
            auto delta = abs_delta;
            if(not is_forward){
                speed = -speed;
                delta = -delta;
            }

            position += iiq32::from_bits(delta.to_bits());
            return {
                position,
                iq20(speed),
                0
            };
        }
        case DemoTrajPattern::Sine:{
            constexpr auto speed = 1_iq16;
            constexpr auto side_amplitude = 1.4_iq16;

            const auto [s,c] = math::sincos(speed * t);
            return {
                make_iiq32(side_amplitude * iq16(s)),
                side_amplitude * speed * iq16(c),
                0
            };
        }

        case DemoTrajPattern::Miniwave:{
            constexpr auto speed = 2_iq16;
            constexpr auto side_amplitude = 0.01_iq16;

            const auto [s,c] = math::sincos(speed * t);
            return {
                make_iiq32(side_amplitude * iq16(s)),
                side_amplitude * speed * iq16(c),
                0
            };
        }
        case DemoTrajPattern::Saw:{
            // const auto [s,c] = math::sincos(speed * t);

            constexpr auto freq = 0.2_iq16;
            constexpr auto amplitude = 5.0_iq16;
            constexpr auto slew_rate = amplitude * freq;
            return {make_iiq32(-iq16(math::frac(t * freq)) * amplitude), -slew_rate, 0};
        }
        case DemoTrajPattern::Stairs:{
            constexpr auto freq = 0.3_iq16;
            constexpr size_t num_steps = 6;
            constexpr auto half_amplitude = 0.4_iq16;
            constexpr auto step = half_amplitude * 2/ num_steps;
            const auto s = iq16(math::sinpu(t * freq));
            return {
                make_iiq32((math::floor(s * (num_steps / 2)) * step)), 
                0,
                0
            };
        }
    }
    //unreachable
    return {0, 0, 0};
};

static constexpr size_t pow2(const size_t x){
    return __builtin_ctz(x);
}

static_assert(pow2(1) == 0);
static_assert(pow2(2) == 1);
static_assert(pow2(16) == 4);


template<size_t FC, size_t Q>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, int32_t> lpf_specified_fc(
    const math::fixed<Q, int32_t> x_state,
    const math::fixed<Q, int32_t> x_new
){
    constexpr auto ALPHA = dsp::calc_lpf_alpha_uq32(CONF_FOC_FREQ, FC).unwrap();
    return lpf_1o(x_state, x_new, ALPHA);
}

constexpr auto ALPHA_100HZ = dsp::calc_lpf_alpha_uq32(CONF_FOC_FREQ, 100).unwrap();
constexpr auto ALPHA_10HZ = dsp::calc_lpf_alpha_uq32(CONF_FOC_FREQ, 10).unwrap();
constexpr auto ALPHA_1HZ = dsp::calc_lpf_alpha_uq32(CONF_FOC_FREQ, 1).unwrap();
constexpr auto ALPHA_01HZ = dsp::calc_lpf_alpha_uq32(CONF_FOC_FREQ * 10, 1).unwrap();

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
DEF_GEN_LPF_FUNC(1)
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




static constexpr auto OBSERVER_PLL_COEFFS = 
    dsp::PllCoeffs::from_fsfc(CONF_FOC_FREQ, CONF_OBSERVER_PLL_FC, 2.0_iq16);

static constexpr auto HFI_PLL_COEFFS = 
    dsp::PllCoeffs::from_fsfc(CONF_FOC_FREQ, CONF_HFI_PLL_FC, 2.0_iq16);






static constexpr auto CURRENT_REGULATOR_CFG = dsp::LrSeriesCurrentRegulatorConfig{
    .fs = CONF_FOC_FREQ,
    .fc = PREFERD_CURRENT_CUTOFF_FREQ,
    .phase_inductance_mh = PHASE_INDUCTANCE_MH,
    .phase_resistance_ohm = PHASE_RESISTANCE_OHM,
};


static constexpr auto PI_CONTROLLER_COEFFS = CURRENT_REGULATOR_CFG.try_into_precomputed().unwrap();

using Ltd2o = dsp::adrc::LinearTrackingDifferentiator<iq16, 2>;

static constexpr size_t HIGHCUTOFF_ENCODER_LTD2O_R = 1200;
[[maybe_unused]] static constexpr auto HIGHCUTOFF_ENCODER_LTD2O = Ltd2o::try_from({
    .fs = CONF_FOC_FREQ, 
    .r = HIGHCUTOFF_ENCODER_LTD2O_R
}).unwrap();




[[maybe_unused]] static constexpr auto CURVE_NLTD_FHAN = dsp::adrc::FhanPrecomputed<iq16>::from({
    .r = CONF_CURVE_X3_LIMIT,
    .h = 0.003_iq16,
});


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
static void process_observer_calc(AllState & state, const FnSwitches fn_switches){


    // if(1){//sensorless observer
    if(false){//sensorless observer
        constexpr auto L = PHASE_INDUCTANCE_MH * uq32(0.001);
        constexpr auto R = PHASE_RESISTANCE_OHM;
        constexpr auto lambda = FLUX_LINKAGE;
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
        //     flux_ob_state.x1_slowhp, x1_delta *CONF_FOC_FREQ, uq32::from_bits(~ALPHA_10HZ.to_bits())
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
        constexpr auto gamma_half_dt = uq16(float(gamma_half) / CONF_FOC_FREQ);

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
}

[[maybe_unused]] __no_inline 
static void process_encoder_cogging_harmonic(AllState & state, const FnSwitches fn_switches){
    #if 0
    {
        //anti cogging encoder harmonic, n = POLE_PAIRS * 6

        // y_hat = hat_theta + hat_b1 * np.sin(n * hat_theta) + hat_b2 * np.cos(n * hat_theta)
        // e = theta_meas - y_hat
        // hat_theta += (z2_f + l1 * e) * dt
        // l2dt = l2 * dt
        // hat_b1 += e * (np.sin(n * hat_theta)) * l2dt
        // hat_b2 += e * (np.cos(n * hat_theta)) * l2dt

        static constexpr size_t POLE_PAIRS = POLE_PAIRS * 2;
        // static constexpr size_t POLE_PAIRS_6X = POLE_PAIRS * 6;
        static constexpr size_t POLE_PAIRS_6X = POLE_PAIRS;
        // [[maybe_unused]] static constexpr iq32 MAX_B = iq32((1.0 / TAU / POLE_PAIRS_6X));
        // [[maybe_unused]] static constexpr iq32 MAX_B = iq32((1.0 / TAU / POLE_PAIRS_6X));
        [[maybe_unused]] static constexpr iq32 MAX_B = iq32((1.0 / POLE_PAIRS_6X));
        [[maybe_unused]] constexpr float MAX_B_f = float(MAX_B);
        // [[maybe_unused]] static constexpr iq20 MAX_B = iq20((1.0 / TAU / POLE_PAIRS) * 0.8);
        // [[maybe_unused]] static constexpr iq32 MAX_B = iq32(1.0);

        auto & peac_state = state.peac_state;

        #if 0
        static constexpr size_t L1 = 115;
        static constexpr size_t L2 = 8;
        #else
        [[maybe_unused]] static constexpr size_t L1 = 115;
        // static constexpr size_t L2 = 2;
        static constexpr auto ALPHA = uq32(0.0001);
        #endif

        // static constexpr uq32 L1DT = uq32(float(L1) / CONF_FOC_FREQ);


        const uq32 harm_turns = uq32::from_bits(peac_state.hat_turns.to_bits() & UINT32_MAX) 
            * (POLE_PAIRS_6X); 

        std::tie(peac_state.harm_s, peac_state.harm_c) = math::sincospu(harm_turns);

        #if 0
        const iq32 harm = iq32::from_bits(
            + intrinsics::mul32hss(peac_state.hat_b1.to_bits(), harm_s.to_bits())
            + intrinsics::mul32hss(peac_state.hat_b2.to_bits(), harm_c.to_bits())
        ) << 1;
        #else
        const iq20 harm = peac_state.hat_b1 * peac_state.harm_s + peac_state.hat_b2 * peac_state.harm_c;
        // const iq32 harm = peac_state.hat_b2 * peac_state.harm_s;
        #endif

        iq20 e = iq20(math::fixed_downcast<20>(state.encoder_abs_position64) - math::fixed_downcast<20>(peac_state.hat_turns))
            - iq20(harm)
            ;

        peac_state.debug.e = e;
        peac_state.harm = harm;
        peac_state.debug.harm_turns = harm_turns;

        // const iq32 e_l1dt = l1dt * e;
        // const iq32 e_l2dt = l2dt * e;

        [[maybe_unused]] const auto now_x2 = state.encoder_ltd_state.x2;

        // peac_state.hat_turns = peac_state.hat_turns + iiq32::from_bits((math::clamp2(now_x2 + e * L1, 200_iq20) * TSAMPLE).to_bits() << 12);
        // peac_state.hat_turns = peac_state.hat_turns + iiq32::from_bits((math::clamp2(now_x2 + e * L1, 200_iq20) * TSAMPLE).to_bits() << 12);
        peac_state.hat_turns = peac_state.hat_turns + iiq32::from_bits((peac_state.hat_speed * TSAMPLE).to_bits() << 12);
        // peac_state.hat_speed = peac_state.hat_speed + (math::clamp2(e * L1, 200_iq20)) * TSAMPLE;
        peac_state.hat_speed = peac_state.hat_speed + math::roundlsb_downcast<20>(
                extended_mul(iq20(2 * (-peac_state.hat_speed)), HIGHCUTOFF_ENCODER_LTD2O.r_dt) 
                + extended_mul(iq20(e), HIGHCUTOFF_ENCODER_LTD2O.r2_dt));

        // static constexpr uq32 ALPHA = calc_lpf;
        // peac_state.hat_b1 += math::mul_roundlsb(e, harm_s * ALPHA);
        peac_state.hat_b1 += math::mul_roundlsb(iq32(peac_state.harm_s * ALPHA), 1000 * e);
        peac_state.hat_b2 += math::mul_roundlsb(iq32(peac_state.harm_c * ALPHA), 1000 * e);
        // peac_state.hat_b1 += math::mul_roundlsb(e, harm_s);
        // peac_state.hat_b2 += math::mul_roundlsb(e, harm_c);

        peac_state.hat_b1 = math::clamp2(peac_state.hat_b1, MAX_B);
        peac_state.hat_b2 = math::clamp2(peac_state.hat_b2, MAX_B);

    }
    
            if(fn_switches.encoder_harmonic_suppression_en){
                encoder_abs_position64 += iiq32::from_bits(state.peac_state.harm.to_bits());
            }
    #endif

}

[[maybe_unused]] __no_inline 
static void process_encoder_pll(
    AllState & state, 
    const FnSwitches fn_switches, 
    iiq32 x1
){
    #if 1
    {
        constexpr size_t pll_fc = 40;
        constexpr size_t zeta = 2;
        constexpr size_t kp = size_t(zeta * 2 * pll_fc);
        constexpr size_t ki = pll_fc * pll_fc;
        constexpr uq32 kidt = TSAMPLE * ki;
        constexpr uq16 kpdt = uq16::from_bits(uint64_t(TSAMPLE.to_bits()) * kp >> 16) ;

        auto & pll_state = state.encoder_pll_state;

        const iq20 e1 = sub_clamp2_downcast_iq20(x1, pll_state.x1, E1_LIMIT);
        pll_state.x2_integral = math::clamp2(pll_state.x2_integral + e1 * kidt, E2_LIMIT);

        pll_state.x2 = pll_state.x2_integral + e1 * kpdt;
        const auto delta_x1 = make_iiq32(pll_state.x2_integral * TSAMPLE) + make_iiq32(e1) * kpdt;
        pll_state.x1 = pll_state.x1 + delta_x1; 
    }
    #endif
}


[[maybe_unused]] __no_inline 
static void process_encoder_ltd(
    AllState & state, 
    const FnSwitches fn_switches, 
    iiq32 x1
){
    #if 1
    {

        auto & state_2o = state.encoder_ltd_state;

        auto & x1_now = state_2o.x1;
        auto & x2_now = state_2o.x2;
        

        const iq20 e1 = sub_clamp2_downcast_iq20(x1, x1_now, E1_LIMIT);

        const iq20 e2 = - x2_now;


        x1_now = x1_now + static_cast<iiq32>(extended_mul(x2_now, TSAMPLE));
        x2_now = x2_now + math::roundlsb_downcast<20>(
            extended_mul(iq20(2 * e2), HIGHCUTOFF_ENCODER_LTD2O.r_dt) 
            + extended_mul(iq20(e1), HIGHCUTOFF_ENCODER_LTD2O.r2_dt));
    }
    #endif
}



#if 0
[[maybe_unused]] __no_inline 
static void process_position_proc(AllState & state, const FnSwitches fn_switches){


    {
        
        auto iterate_openloop_angle = [&]{


            state.openloop_elec_speed = 5.80_iq16;

            state.openloop_elec_angle = make_angular_from_turns(
                state.openloop_elec_angle.to_turns() 
                + uq32(TSAMPLE * state.openloop_elec_speed)
            );
        };



        std::tie(state.elec_angle, state.elec_speed) = [&] -> std::tuple<Angular<uq32>, iq20>{
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
                    const auto elec_speed = (iq20)state.observer_pll_state.angluar_speed_integral.to_turns();
                    return {elec_angle, elec_speed};
                }
                
                
                case ElecAngleSource::Hfi:{
                    const auto elec_angle = state.hfi_elec_angle;
                    const auto elec_speed = iq20(0);
                    return {elec_angle, elec_speed};
                }
                
                case ElecAngleSource::MagEncoder:
                case ElecAngleSource::AbzEncoder:
                

            }
            __builtin_unreachable();
        }();

    }
};

#endif


[[maybe_unused]] 
static void process_traj_preshape(
    AllState & state, 
    FnSwitches fn_switches,
    const iiq32 traj_x1,
    const iq20 traj_x2,
    [[maybe_unused]] const iq16 traj_x3
){
    const auto traj_smooth_method = fn_switches.traj_smooth_method;
    const bool traj_frontend_smooth_en = (traj_smooth_method != TrajSmoothMethod::Disabled);
    
    if(traj_frontend_smooth_en){
        //启用此项 为轨迹规划器提供准确的前馈速度 使得规划输出与原始位置信号几乎零迟滞

        const bool use_traj_x2 = (traj_smooth_method == TrajSmoothMethod::UseX1AndX2);

        auto & now_state = state.traj_smooth_state;

        auto & x1_now = now_state.x1;
        auto & x2_now = now_state.x2;

        auto ref_x1 = traj_x1;
        auto ref_x2 = (use_traj_x2 ? traj_x2 : iq20(0));

        const iq20 e1 = sub_clamp2_downcast_iq20(ref_x1, x1_now, E1_LIMIT);
        const iq20 e2 = math::clamp2(ref_x2 - x2_now, E2_LIMIT);

        x1_now = x1_now + static_cast<iiq32>(extended_mul(x2_now, TSAMPLE));
        x2_now = x2_now + math::roundlsb_downcast<20>(
            extended_mul(iq20(2 * e2), HIGHCUTOFF_ENCODER_LTD2O.r_dt) 
            + extended_mul(iq20(e1), HIGHCUTOFF_ENCODER_LTD2O.r2_dt));
    }else{
        state.traj_smooth_state.x1 = traj_x1;
        //无前馈速度
        state.traj_smooth_state.x2 = 0;
    }
}


[[maybe_unused]]
static void process_traj_shape(
    AllState & state, 
    FnSwitches fn_switches,
    const iiq32 hp_traj_x1,
    const iq20 hp_traj_x2,
    [[maybe_unused]] const iq16 hp_traj_x3
){
    const auto traj_smooth_method = fn_switches.traj_smooth_method;
    //基于fhan的梯形速度规划
    auto & curve_state = state.curve_state;

    const bool do_comp_traj_x1 = traj_smooth_method == TrajSmoothMethod::UseX1AndZero;

    //使用终值定理得到无静差时补偿量为x2 * 2/r
    const iq16 x1_traj_comp = do_comp_traj_x1 ? iq16(hp_traj_x2 * HIGHCUTOFF_ENCODER_LTD2O.two_inv_r) : iq16(0);
    const iiq32 x1_traj = hp_traj_x1 + make_iiq32(x1_traj_comp);
    auto e1 = sub_clamp2_downcast_iq20(x1_traj, curve_state.x1, E1_LIMIT);

    const bool auto_curve_retrack_en = fn_switches.auto_curve_retrack_en;
    const bool do_curve_retrack_when_needed = auto_curve_retrack_en;
    static constexpr auto RETRACK_E1_THRESHOLD = 0.4_iq20;
    static constexpr auto RETRACK_LEAD_X1 = 0.1_iq20;

    if(do_curve_retrack_when_needed){
        const auto x1_encoder = state.encoder_ltd_state.x1;
        if(math::abs(sub_clamp2_downcast_iq20(curve_state.x1, x1_encoder, 100)) > RETRACK_E1_THRESHOLD){
            const iiq32 x1_curve_retrack = math::step_to(x1_encoder, x1_traj, make_iiq32(RETRACK_LEAD_X1));
            const auto x2_curve_retrack = state.encoder_ltd_state.x2;

            curve_state.x1 = x1_curve_retrack;
            curve_state.x2 = x2_curve_retrack;
            state.retrack_count++;
            e1 = sub_clamp2_downcast_iq20(x1_traj, x1_curve_retrack, 100);
        }
    }
    
    const iq20 e2 = math::clamp2(hp_traj_x2 - curve_state.x2, E2_LIMIT);

    const auto u = CURVE_NLTD_FHAN({iq16(e1), iq16(e2)});

    const auto next_x1 = curve_state.x1 + iiq32(extended_mul(curve_state.x2, TSAMPLE));
    const auto next_x2 = math::clamp2(curve_state.x2 + iq20(u) * TSAMPLE, CONF_CURVE_X2_LIMIT);

    const auto delta_x2 = next_x2 - curve_state.x2;

    curve_state.x1 = next_x1;
    curve_state.x2 = next_x2;
    // curve_state.x3 = math::closer_to_zero(iq16(u), lpf_2000hz(curve_state.x3, iq16(delta_x2) * CONF_FOC_FREQ));
    curve_state.x3 = iq16(delta_x2) * CONF_FOC_FREQ;
    curve_state.u = u;
}


[[maybe_unused]] __no_inline 
static void process_mechanical_loop(
    AllState & state, 
    FnSwitches fn_switches,
    const iiq32 curve_x1,
    const iq20 curve_x2,
    const iq16 curve_x3
){
    //#region 力矩转电流

    const auto now_x2 = state.encoder_ltd_state.x2;
    const auto now_x1 = state.encoder_ltd_state.x1;

    constexpr auto torque_curr_step_limit = TORQUE_CURR_STEP_LIMIT;
    constexpr auto torque_curr_limit = TORQUE_CURR_LIMIT;

    const auto loop_wiring = fn_switches.loop_wiring;

    const iq20 ka = 0.0012_iq20;
    const auto x3comp_torque_curr = ka * curve_x3 * fn_switches.interia_forwardfeedback_en;

    const iq20 kf = 0.01_iq20;
    const auto x2comp_torque_curr = kf * curve_x2 * fn_switches.damping_forwardfeedback_en;

    state.torque_curr_x3comp = x3comp_torque_curr;
    state.torque_curr_x2comp = x2comp_torque_curr;

    switch(loop_wiring){
        case LoopWiring::SeriesPi:{
            #if 0
            //GIM4310
            const iq20 kpp_normal = 12;
            const iq20 kpp_big = 20;
            const iq20 kpp = fn_switches.override_big_position_kp ? kpp_big : kpp_normal;
            // const iq20 kp = 1.2_iq20;
            constexpr iq20 kp = 0.6_iq20;
            constexpr iq20 ki = 6.66_iq20;
            #else
            //Jc4310
            const iq20 kpp_normal = 20;
            const iq20 kpp_big = 30;
            const iq20 kpp = fn_switches.override_big_position_kp ? kpp_big : kpp_normal;
            // const iq20 kp = 1.2_iq20;
            constexpr iq20 kp = 0.43_iq20;
            constexpr iq20 ki = 14.66_iq20;
            #endif
            constexpr auto ki_discrete = ki / CONF_FOC_FREQ;

            const iq20 ref_x2 = iq20(math::mul_roundlsb_clamp2(fixed_downcast<20>(curve_x1 - now_x1), kpp, E2_LIMIT));

            state.pi_ref_x2 = ref_x2;
            const iq20 e2 = math::clamp2((ref_x2 + curve_x2) - now_x2, E2_LIMIT);
            state.pi_e2 = e2;

            auto desired_torque_curr_cmd = state.torque_curr_integral 
                + math::mul_roundlsb(kp, e2)
                + x2comp_torque_curr
                + x3comp_torque_curr
            ;

            desired_torque_curr_cmd = math::clamp2(desired_torque_curr_cmd, torque_curr_limit);
            desired_torque_curr_cmd = (3 * state.torque_curr_cmd + desired_torque_curr_cmd) >> 2;
            state.torque_curr_cmd = math::step_to(state.torque_curr_cmd, desired_torque_curr_cmd, torque_curr_step_limit);

            auto torque_curr_integral_delta = 
                e2 * ki_discrete
                + (state.torque_curr_cmd - desired_torque_curr_cmd)
            ;
            auto torque_curr_integral = state.torque_curr_integral + math::clamp2(
                torque_curr_integral_delta,
                torque_curr_step_limit
            );

            state.torque_curr_integral = math::clamp2(torque_curr_integral, torque_curr_limit);
            break;
        }

        case LoopWiring::Mit:{
            const iq20 kp = 12.7_iq16;
            const iq20 kd = 0.26_iq16;

            const iq20 e1 = sub_clamp2_downcast_iq20(curve_x1, now_x1, E1_LIMIT);
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
    }

    state.torque_curr_veryslowlp = lpf_10hz(state.torque_curr_veryslowlp, state.torque_curr_cmd);
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

            constexpr auto L = PHASE_INDUCTANCE_MH * uq32(0.001);
            // constexpr auto R = PHASE_RESISTANCE_OHM;
            const auto dec_factor = kp * phi_s - 6 * elec_omega_rads * L;

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


[[maybe_unused]] __no_inline static 
void process_deadcomp_generate(
    AllState & state, 
    const FnSwitches fn_switches
){
    // const bool deadtime_comp_en = true;
    const bool deadtime_comp_en = fn_switches.deadtime_compensate_en;

    if(deadtime_comp_en){
        // https://www.zhihu.com/question/270446098/answer/3215795384
        // 《SVPWM逆变器死区补偿的研究与实现》 魏凯

        const auto uvw_curr_fastlp = state.uvw_curr_fastlp;
        [[maybe_unused]] const auto weak_flip_threshold = CURRENT_AMPS_PER_ADC_LSB * 3;
        [[maybe_unused]] const auto strong_flip_threshold = CURRENT_AMPS_PER_ADC_LSB * 6;
        static constexpr auto DEADTIME_COMP_DUTYCYCLE = iq16(
            (CONF_DEADTIME_NANOS.count() * CONF_FOC_FREQ * 1e-9)
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

        state.uvw_dutycycle_deadcomp = uvw_dutycycle_deadcomp;
        state.uvw_dutycycle_genout += uvw_dutycycle_deadcomp;
    }
}

[[maybe_unused]] __no_inline static 
void process_hfi_generate(
    AllState & state, 
    const FnSwitches fn_switches
){
    auto & hfi_state = state.hfi_state;


    // const bool hfi_enabled = true;
    const auto hfi_method = fn_switches.hfi_method;
    const bool hfi_enabled = hfi_method != HfiMethod::Disabled;
    const bool hfi_use_spin = hfi_method == HfiMethod::Spin;

    if(hfi_enabled){
        auto & table = SINCOS_32STEP_TABLE;
        static constexpr size_t TABLE_SIZE = std::tuple_size_v<std::decay_t<decltype(table)>>;
        static_assert(std::has_single_bit(TABLE_SIZE));

        const auto table_idx_mask = TABLE_SIZE - 1;
        const auto hfi_modu_depth = CONF_HFI_MODU_DEPTH_LIMIT;


        auto calc_spin_hfi_dutycycle = [&] -> AlphaBetaCoord<iq20>{
            const auto lg2_len_hfi_samples = 5;
            const auto len_hfi_samples = 1 << lg2_len_hfi_samples;

            const auto table_fact = TABLE_SIZE / len_hfi_samples;
            
            const auto now_hfi_idx = hfi_state.hfi_idx;
            if(now_hfi_idx >= len_hfi_samples) __builtin_unreachable();
            const auto [s_bin1,c_bin1] = table[(now_hfi_idx * table_fact) & table_idx_mask];
            const auto [s_bin2,c_bin2] = table[(now_hfi_idx * table_fact * 2) & table_idx_mask];

            const auto hfi_response = dot2v2(
                state.alphabeta_curr_raw.alpha, c_bin1,
                state.alphabeta_curr_raw.beta, s_bin1
            );

            if(hfi_state.hfi_is_neg_samp){
                const auto di = hfi_response - hfi_state.hfi_response;
                hfi_state.hfi_response = hfi_response;

                auto next_hfi_idx = now_hfi_idx + 1;


                if(next_hfi_idx >= len_hfi_samples){
                    next_hfi_idx = 0;

                    hfi_state.spinhfi_bin0_real_response = hfi_state.spinhfi_bin0_real_response_acc >> lg2_len_hfi_samples;
                    hfi_state.spinhfi_bin0_real_response_acc = 0;

                    hfi_state.spinhfi_bin2_real_response = hfi_state.spinhfi_bin2_real_response_acc >> lg2_len_hfi_samples;
                    hfi_state.spinhfi_bin2_real_response_acc = 0;

                    hfi_state.spinhfi_bin2_imag_response = hfi_state.spinhfi_bin2_imag_response_acc >> lg2_len_hfi_samples;
                    hfi_state.spinhfi_bin2_imag_response_acc = 0;

                    hfi_state.spinhfi_bin2_real_response_slowlp = lpf_100hz(hfi_state.spinhfi_bin2_real_response_slowlp, hfi_state.spinhfi_bin2_real_response);
                    hfi_state.spinhfi_bin2_imag_response_slowlp = lpf_100hz(hfi_state.spinhfi_bin2_imag_response_slowlp, hfi_state.spinhfi_bin2_imag_response);
                }else{
                    hfi_state.spinhfi_bin0_real_response_acc += di;

                    hfi_state.spinhfi_bin2_real_response_acc += di * c_bin2;
                    hfi_state.spinhfi_bin2_imag_response_acc += di * s_bin2;
                }


                hfi_state.hfi_idx = next_hfi_idx;
            }else{
                hfi_state.hfi_response = hfi_response;
            }


            auto bin2_real_response = hfi_state.spinhfi_bin2_real_response;
            auto bin2_imag_response = hfi_state.spinhfi_bin2_imag_response;

            if(math::abs(state.hfi_pll_state.angluar_speed_integral.to_turns()) > 10){
                bin2_real_response -= hfi_state.spinhfi_bin2_real_response_slowlp;
                bin2_imag_response -= hfi_state.spinhfi_bin2_imag_response_slowlp;
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
                - hfi_state.prev_hfi_lap_angle2x.cast_inner<iq16>()).signed_normalized();

            hfi_state.prev_hfi_lap_angle2x = now_hfi_lap_angle2x;
            hfi_state.hfi_multilap_angle2x = hfi_state.hfi_multilap_angle2x + hfi_diff_angle2x;

            auto hfi_elec_angle = make_angular_from_turns(uq32(math::frac(hfi_state.hfi_multilap_angle2x.to_turns() >> 1)));
            state.hfi_elec_angle = hfi_elec_angle;

            if(hfi_state.hfi_is_neg_samp){
                hfi_state.hfi_is_neg_samp = false;
                return AlphaBetaCoord<iq20>{
                    .alpha = hfi_modu_depth * c_bin1,
                    .beta = hfi_modu_depth * s_bin1,
                };
            }else{
                hfi_state.hfi_is_neg_samp = true;
                return AlphaBetaCoord<iq20>{
                    .alpha = (-hfi_modu_depth) * c_bin1,
                    .beta = (-hfi_modu_depth) * s_bin1,
                };
            }
        };




        auto calc_pulse_hfi_dutycycle = [&] -> AlphaBetaCoord<iq20>{
            const auto lg2_len_hfi_samples = 5;
            const auto len_hfi_samples = 1 << lg2_len_hfi_samples;

            const auto table_fact = TABLE_SIZE / len_hfi_samples;

            // const Angular<uq32> est_angle = state.hfi_pll_state.angle;
            #if 0
            const Angular<uq32> est_angle = Zero;
            const auto est_angle_sincos = math::Rotation2<iq31>::from_angle(est_angle);
            #else
            const Angular<uq32> est_angle = state.hfi_pll_state.angle;
            const auto est_angle_sincos = math::Rotation2<iq31>::from_angle(est_angle);

            #endif


            const auto now_hfi_idx = hfi_state.hfi_idx;
            if(now_hfi_idx >= len_hfi_samples) __builtin_unreachable();
            const auto [s_bin1,c_bin1] = table[(now_hfi_idx * table_fact) & table_idx_mask];

            const auto est_dq_curr = state.alphabeta_curr_raw.inv_rotate(est_angle_sincos);

            // if(hfi_state.hfi_is_neg_samp){
                // const auto di = hfi_response - hfi_state.hfi_response;
                // hfi_state.hfi_response = hfi_response;

                auto next_hfi_idx = now_hfi_idx + 1;
                if(next_hfi_idx >= len_hfi_samples){
                    next_hfi_idx = 0;
                }else{

                }

                // hfi_state.pulsehfi_q_response = di * c_bin1;
                hfi_state.pulsehfi_q_response = lpf_500hz(hfi_state.pulsehfi_q_response, est_dq_curr.q * c_bin1);
                // hfi_state.pulsehfi_q_response = est_dq_curr.q / est_dq_curr.d;
                // hfi_state.pulsehfi_q_response = est_dq_curr.d;


                hfi_state.hfi_idx = next_hfi_idx;

            // }else{
            //     hfi_state.hfi_response = hfi_response;
            // }


            HFI_PLL_COEFFS.iterate_err(state.hfi_pll_state, hfi_state.pulsehfi_q_response * 0.9_iq16);
            auto hfi_elec_angle = state.hfi_pll_state.angle;
            state.hfi_elec_angle = hfi_elec_angle + make_angular_from_turns(0.5_uq32);


            const auto dq_dutycycle = DqCoord<iq20>{
                // hfi_modu_depth * (hfi_state.hfi_is_neg_samp ? c_bin1 : -c_bin1),
                hfi_modu_depth * (c_bin1),
                0.0_iq20
            };

            hfi_state.hfi_is_neg_samp = !hfi_state.hfi_is_neg_samp;

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

        state.alphabeta_dutycycle_final += state.alphabeta_dutycycle_hfi;
    }

}

__no_inline static 
void process_current_loop(
    AllState & state, 
    const FnSwitches fn_switches, 
    Angular<uq32> elec_angle,
    iq16 elec_speed,
    iq20 tc_ref
){
    const auto elec_omega_rads = elec_speed * iq16(TAU);

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
        const bool do_fw = fn_switches.mtpv_en;


        constexpr auto ld_lq_diff = Q_AXIS_INDUCTANCE_MH - D_AXIS_INDUCTANCE_MH;

        constexpr auto lambda = 1000 * FLUX_LINKAGE;
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
        if(do_fw){
        }

        state.dq_curr_ref.d = d_curr_ref;
        state.dq_curr_ref.q = q_curr_ref;

        state.alphabeta_curr_ref = state.dq_curr_ref.rotate(elec_sincos);
        state.uvw_curr_ref = state.alphabeta_curr_ref.to_uvw();

        state.backcalc_torque_curr = q_curr_ref * (1 + ld_lq_diff_by_lambda * d_curr_ref);

    }

    {
        iq20 d_volt_decouple = 0;
        iq20 q_volt_decouple = 0;

        const bool is_speed_stable = true;
        const auto omega_rads = elec_omega_rads * is_speed_stable;
        
        const auto phase_ind = PHASE_INDUCTANCE_MH * uq32(0.001);
        
        
        const bool cross_decoupling_enabled = fn_switches.cross_decoupling_en;

        if(cross_decoupling_enabled){
            d_volt_decouple -= phase_ind * state.dq_curr_raw.q * omega_rads;
            q_volt_decouple += phase_ind * state.dq_curr_raw.d * omega_rads;
        }

        const bool bemf_decoupling_enabled = fn_switches.bemf_decoupling_en;

        if(bemf_decoupling_enabled){
            q_volt_decouple += FLUX_LINKAGE * omega_rads;
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

    // process_hfi_generate();

    state.alphabeta_dutycycle_final = (state.dq_volt_ctrl * inv_busbar_volt_3by2).rotate(elec_sincos);
    state.alphabeta_volt_final = state.alphabeta_dutycycle_final * BUSBAR_VOLT;

    {
        state.uvw_dutycycle_genout = SVM(state.alphabeta_dutycycle_final);
        process_deadcomp_generate(state, fn_switches);
    }
};


template<typename Derived, typename Record>
struct [[nodiscard]] alignas(size_t) ProfilerBase {
    Record* data;
    size_t length;
    size_t capacity;

    template<typename Allocator>
    static Option<Derived &> try_from_arena_allocate(Allocator& alloc, size_t capacity) noexcept {
        const size_t nbytes = sizeof(Derived) + capacity * sizeof(Record);
        auto* allocated = static_cast<uint8_t*>(alloc.allocate(nbytes));
        if (!allocated) return None;
        // 对齐偏移（假设 allocated 已满足最大对齐）
        const size_t offset = (sizeof(Derived) + alignof(Record) - 1) & ~(alignof(Record) - 1);
        auto* p_this = new (allocated) Derived{};
        p_this->data = reinterpret_cast<Record*>(allocated + offset);
        p_this->length = 0;
        p_this->capacity = capacity;
        return Some(p_this);
    }

    const Record& nth_record(size_t index) const noexcept {
        if (index >= length) __builtin_abort();
        return data[index];
    }

    const Option<Record&> try_nth_record(size_t index) const noexcept {
        if (index >= length) return None;
        return &data[index];
    }

    void reset(){length = 0;}

    Record * begin() noexcept {return data;}
    Record * end() noexcept {return data + length;}
    const Record * begin() const noexcept {return data;}
    const Record * end() const noexcept {return data + length;}

protected:
    // 禁止复制/移动
    ProfilerBase() = default;
    ~ProfilerBase() = default;
    ProfilerBase(const ProfilerBase&) = delete;
    ProfilerBase& operator=(const ProfilerBase&) = delete;
    ProfilerBase(ProfilerBase&&) = delete;
    ProfilerBase& operator=(ProfilerBase&&) = delete;
};


using ShortString8 = ThriftyInlineString<8>;


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

struct [[nodiscard]] alignas(size_t) MyRecord final{
    using Id = ShortString8;
    
    ShortString8 name;
    TimerTick entry;
    TimerTick exit;

    MyRecord(ShortString8 _name, TimerTick _entry, TimerTick _exit):
        name(_name), entry(_entry), exit(_exit){;}

    __attribute__((always_inline)) 
    MyRecord(const MyRecord & other){
        const auto p_dst = reinterpret_cast<uint32_t *>(this);
        const auto p_src = reinterpret_cast<const uint32_t *>(&other);
        p_dst[0] = p_src[0];
        p_dst[1] = p_src[1];
        p_dst[2] = p_src[2];
        p_dst[3] = p_src[3];
    }
        
    friend OutputStream & operator <<(OutputStream & ostm, MyRecord & self){
        return ostm << self.name.view() << tmrticks_to_us(self.entry) << tmrticks_to_us(self.exit);
    }
};

struct MyProfiler : public ProfilerBase<MyProfiler, MyRecord> {
    Result<void, void> try_push_record(MyRecord&& record) {
        if (length >= capacity) return Err();
        new (&data[length]) MyRecord(std::move(record));  // 构造
        ++length;
        return Ok();
    }
};




__no_inline auto  profiler_push_record(MyProfiler & profiler, MyRecord && record){
    return profiler.try_push_record(std::move(record));
}

static constexpr bool judge_is_disconn(const iq20 meas, const iq20 ref){
    const auto abs_ref = math::abs(ref);
    if(abs_ref < CURRENT_NOISE_STDVAR) return false;
    const auto abs_meas = math::abs(meas);
    return (abs_meas * 30 < abs_ref) and (abs_meas < CURRENT_NOISE_STDVAR);
}




static constexpr std::partial_ordering compare_bvalue(const int32_t bvalue){
    static constexpr int32_t HI_THRESHOLD = ADC_MIDPOINT_BVALUE + CONF_ADC_MIDPOINT_OFFSET_BVALUE_TOLERANCE;
    static constexpr int32_t LO_THRESHOLD = ADC_MIDPOINT_BVALUE - CONF_ADC_MIDPOINT_OFFSET_BVALUE_TOLERANCE;
    if(bvalue >= HI_THRESHOLD) return std::partial_ordering::greater;
    if(bvalue <= LO_THRESHOLD) return std::partial_ordering::less;
    return std::partial_ordering::equivalent;
};


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

            {hal::AdcChannelSelection::TEMP, hal::AdcSampleCycles::T28_5},  
            #if 1
            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T13_5},
            {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T13_5},
            {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T13_5},  
            #else
            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T28_5},  
            #endif

            // {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T7_5},
            // {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T7_5},
            // {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T7_5},

        },
        {}
    );

    
    lld::adc_set_injected_trigger(ADC1, hal::AdcInjectedTrigger::T1CC4);
    lld::adc_enable_auto_inject(ADC1, DISEN);
    lld::adc_cmd(ADC1, EN);
}

static void setup_timer(){
    auto & timer = hal::timer1;
    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__A7_B0_B1,
        // .count_freq = hal::NearestFreq(CONF_FOC_FREQ * 2),
        .count_freq = hal::timer::ArrAndPsc{TIMER_ARR_VALUE,1-1},
        // .count_mode = hal::TimerCountMode::CenterAlignedDualTrig,
        // .count_mode = hal::TimerCountMode::CenterAligned,
        .count_mode = hal::TimerCountMode::CenterAlignedDownTrig,
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

    timer.configure_bdtr(CONF_DEADTIME_NANOS, Default);
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
        static_cast<int32_t>(inst->IDATAR2),
        static_cast<int32_t>(inst->IDATAR3),
        static_cast<int32_t>(inst->IDATAR4)
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
    // drv8323_gain_pin_.inflt();//20x
    drv8323_gain_pin_.outpp(HIGH);//40x


    //使用更小的拉灌电流有助于减小mcu侧的adc毛刺
    // drv8323_idrive_pin_.outpp(HIGH);//Sink 2A / Source1A
    drv8323_idrive_pin_.inpu();
    // drv8323_idrive_pin_.inflt();//Sink 240mA/ Source 120mA
    // drv8323_idrive_pin_.outpp(LOW);


    drv8323_vds_pin_.outpp(LOW); //10A保护
    // drv8323_vds_pin_.outpp(HIGH); //dangerous no ocp protect!!!!
}


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
    using MagEncoder = drivers::VCE2755;


    // struct [[nodiscard]] FutureAngle{
    //     MagEncoder mag_encoder_;

    //     Angular<uq32> get(){
    //         return mag_encoder_.get_angle().examine().parse().unwrap();
    //     }
    // };

    auto mag_encoder_ = MagEncoder{
        &spi,
        spi.allocate_cs_pin(&mag_encoder_cs_pin_)
            .unwrap()
    };

    mag_encoder_.init(Default).examine();
    mag_encoder_.set_direction(CW).examine();
    mag_encoder_.set_filter_bandwidth(MagEncoder::FilterBandwidth::_8BW0).examine();


    auto mag_encoder_poll_conversion = [&] -> void{
    };

    auto mag_encoder_waitfor_angle = [&] -> Angular<uq32>{
        return mag_encoder_.get_angle().examine().parse().unwrap();
    };

    for(size_t i = 0; i < 100; i++){
        mag_encoder_poll_conversion();
        auto angle = mag_encoder_waitfor_angle();
        (void)angle;
        clock::delay(100us);
    }
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
        const auto millis_u32 = uint32_t(clock::millis().count());
        led_red_pin_ = BoolLevel::from((millis_u32 % 200u) > 100);
        led_blue_pin_ = BoolLevel::from((millis_u32 % 400u) > 200);
        led_green_pin_ = BoolLevel::from((millis_u32 % 800u) > 400);
    };


    // #endregion


    auto op_flags_ = OpFlags::from_default();
    op_flags_.dc_calibrate_unready = true;

    op_flags_.sideshaft_calibrate_unready = false;
    // op_flags_.sideshaft_calibrate_unready = true;

    auto fn_switches_ = FnSwitches::from_default();
    fn_switches_.current_harmonic_suppression_en = 0;
    fn_switches_.elec_angle_source = ElecAngleSource::MagEncoder;
    fn_switches_.loop_wiring = LoopWiring::SeriesPi;
    fn_switches_.override_big_position_kp = false;
    
    // fn_switches_.traj_smooth_method = TrajSmoothMethod::UseX1AndZero;
    // fn_switches_.traj_smooth_method = TrajSmoothMethod::Disabled;
    fn_switches_.traj_smooth_method = TrajSmoothMethod::UseX1AndX2;

    fn_switches_.auto_curve_retrack_en = true;
    fn_switches_.damping_forwardfeedback_en = false;
    fn_switches_.interia_forwardfeedback_en = false;
    fn_switches_.sideshaft_compensate_en = true;

    static constexpr size_t HEAP_ARENA_SIZE = 4096;
    auto p_arena_resource = std::make_unique<uint8_t[]>(HEAP_ARENA_SIZE);
    auto arena_allocater = mem::ArenaAllocater::from(std::span(p_arena_resource.get(), HEAP_ARENA_SIZE));


    auto & all_state_ = *reinterpret_cast<AllState *>(arena_allocater.allocate(sizeof(AllState)));
    all_state_.reset();
    all_state_.torque_curr_cmd = 0.0_iq20;

    // auto & profiler_ = MyProfiler::try_from_arena_allocate(alloc, 8).unwrap();

    // profiler_.reset();
    // profiler_push_record(profiler_, MyRecord{ShortString8::try_from_cstr("MyTick00").unwrap(), get_timer_tick(), get_timer_tick()}).unwrap();
    // profiler_push_record(profiler_, MyRecord{ShortString8::try_from_cstr("wtf").unwrap(), get_timer_tick(), get_timer_tick()}).unwrap();
    auto foc_loop = [&]{

        static constexpr size_t SIDESHAFT_EPS_TABLE_LENGTH = POLE_PAIRS * 6;
        static_assert(SIDESHAFT_EPS_TABLE_LENGTH < ENCODER_SIDESHAFT_EPS_TABLE_CAPACITY);
        static constexpr size_t SIDESHAFT_CALIBRATE_OVERSAMPLES = 16;
        static constexpr size_t SIDESHAFT_CALIBRATE_STEPS_PER_ELEMENT = 256 * 2;
        static constexpr size_t SIDESHAFT_CALIBRATE_STEPS_PER_SAMPLE = SIDESHAFT_CALIBRATE_STEPS_PER_ELEMENT / SIDESHAFT_CALIBRATE_OVERSAMPLES;
        static constexpr size_t CALIBRATE_REVS_PER_DIRECTION = 2;
        static constexpr size_t SIDESHAFT_CALIBRATE_STEPS_PER_REV = (SIDESHAFT_CALIBRATE_STEPS_PER_ELEMENT * SIDESHAFT_EPS_TABLE_LENGTH);
        
        auto  & state = all_state_;

        //do clone, avoid external modify during isr
        const volatile uint32_t fn_switches_bits = std::bit_cast<uint32_t>(fn_switches_);
        const auto fn_switches = std::bit_cast<FnSwitches>(uint32_t(fn_switches_bits));

        // const volatile uint32_t op_flags_bits = std::bit_cast<uint32_t>(op_flags_);
        // const auto op_flags = std::bit_cast<FnSwitches>(uint32_t(op_flags_bits));
        auto & op_flags = op_flags_;



        const bool is_sideshaft_calibrate_done = !op_flags.sideshaft_calibrate_unready;
        const bool do_sideshaft_calibrate = (fn_switches.sideshaft_compensate_en) and (!is_sideshaft_calibrate_done);


        mag_encoder_poll_conversion();



        auto encoder_position_raw = mag_encoder_waitfor_angle().to_turns();

        auto is_adc_injected_converstion_end = [&]{
            static constexpr uint32_t JEOC_FLAG_MASK = 1u << 2;
            if(ADC1->STATR & JEOC_FLAG_MASK){
                ADC1->STATR = ADC1->STATR & (~JEOC_FLAG_MASK);
                return true;
            }
            return false;
        };


        #if 0
        {
            static constexpr size_t LG2_SIMULATED_ENCODER_RESOLUTION = 12;
            static constexpr uint32_t MASK = ((1u << LG2_SIMULATED_ENCODER_RESOLUTION) - 1) << (32 - LG2_SIMULATED_ENCODER_RESOLUTION);
            encoder_position_raw.bits &= MASK;
        }
        #endif



        auto warp_encoder_err = [](const uq32 ref, const uq32 meas) -> iq32{
            return ((iq32(ref) - iq32(meas)) * POLE_PAIRS) * uq32(1.0 / POLE_PAIRS);
        };

        auto steps_to_turns = [](const size_t steps) -> uq32{
            const size_t rem = steps % SIDESHAFT_CALIBRATE_STEPS_PER_REV;
            constexpr uq32 FACTOR = uq32::from_bits(uint32_t(float(1ull << 32) / SIDESHAFT_CALIBRATE_STEPS_PER_REV) + 1);
            return rem * FACTOR;
        };



        auto calc_encoder_eps = [](const iq32 * table_data, const uq32 raw_turns) -> iq32{
            auto index = intrinsics::mul32hu(SIDESHAFT_EPS_TABLE_LENGTH, raw_turns.to_bits());
            const auto frac = (raw_turns * SIDESHAFT_EPS_TABLE_LENGTH);

            auto next_index = warp_index(index + 1, SIDESHAFT_EPS_TABLE_LENGTH);

            static constexpr size_t RIGHT_SHIFTS = pow2(SIDESHAFT_CALIBRATE_OVERSAMPLES * CALIBRATE_REVS_PER_DIRECTION);
            const auto & now_element = table_data[index];
            const auto & next_element = table_data[next_index];
            
            const auto now_element_value = now_element;
            const auto next_element_value = next_element;

            const auto table_eps = lerp_fixed_uq32(now_element_value, next_element_value, frac) >> RIGHT_SHIFTS;
            return table_eps;
        };


        auto correct_encoder_position = [&](const iq32 * table_data, uq32 raw_turns) -> uq32{
            if(is_sideshaft_calibrate_done & fn_switches.sideshaft_compensate_en){
                const auto table_eps = calc_encoder_eps(table_data, raw_turns);
                return raw_turns + table_eps;
            }else{
                return raw_turns;
            }
        };

        
        const auto encoder_position = correct_encoder_position(
            state.encoder_calibrate_state.eps_table.data(), encoder_position_raw);
            
        #if 0

        auto calc_encoder_correct_method_signature = [&] -> uint8_t{
            return 0xf0 | is_sideshaft_calibrate_done;
        };


        const auto encoder_correct_method_signature = calc_encoder_correct_method_signature();

        if(state.encoder_correct_method_signature != encoder_correct_method_signature){
            state.encoder_initial_position = correct_encoder_position(
                state.encoder_calibrate_state.eps_table.data(), state.encoder_initial_position_raw);
            
            state.encoder_correct_method_signature = encoder_correct_method_signature;
        }
        #else
            
        state.encoder_initial_position = correct_encoder_position(
            state.encoder_calibrate_state.eps_table.data(), state.encoder_initial_position_raw);
        #endif


        if(not state.is_encoder_initial_position_recorded){
            state.encoder_initial_position_raw = encoder_position_raw;
            state.encoder_abs_position64 = iiq32::from_bits(encoder_position.to_bits());
            state.is_encoder_initial_position_recorded = true;
        }else{
            state.encoder_abs_position64 = uq32_wrapped_update(
                state.encoder_abs_position64, encoder_position);
        }


        state.encoder_rel_position64 = state.encoder_abs_position64 
            - iiq32::from_bits(state.encoder_initial_position.to_bits());

        process_encoder_ltd(state, fn_switches, state.encoder_rel_position64);
        process_encoder_pll(state, fn_switches, state.encoder_abs_position64);

        auto encoder_abs_position32 = iiq32_crop_frac(state.encoder_abs_position64);

        // const auto elec_align_offset = 0.946429_uq32;
        const auto elec_align_offset = 0.80_uq32;
        // const auto elec_align_offset = 0.75_uq32;
        const auto mech_angle = Angular<uq32>::from_turns((encoder_abs_position32));
        state.sensed_elec_angle = (mech_angle * POLE_PAIRS) + Angular<uq32>::from_turns(elec_align_offset);

        const auto mech_speed = state.encoder_ltd_state.x2;
        state.sensed_elec_speed = iq16(mech_speed) * POLE_PAIRS;


        state.elec_angle = state.sensed_elec_angle;
        state.elec_speed = state.sensed_elec_speed;


        while(not is_adc_injected_converstion_end());
        state.uvw_adc_bvalue = get_adc_uvw_bvalue();
        process_current_sense(state, fn_switches, state.uvw_adc_bvalue);

        state.encoder_get_done_tick = get_timer_tick();

        if(op_flags.dc_calibrate_unready){
            auto & dc_state = state.dc_calibrate_state;
            dc_state.uvw_bvalue_offset_acc = {
                std::get<0>(dc_state.uvw_bvalue_offset_acc) + int32_t(std::get<0>(state.uvw_adc_bvalue)),
                std::get<1>(dc_state.uvw_bvalue_offset_acc) + int32_t(std::get<1>(state.uvw_adc_bvalue)),
                std::get<2>(dc_state.uvw_bvalue_offset_acc) + int32_t(std::get<2>(state.uvw_adc_bvalue))
            };
            dc_state.dc_cal_cnt++;
            if(dc_state.dc_cal_cnt >= DC_CAL_TIMES){
                dc_state.uvw_bvalue_offset = {
                    int32_t(std::get<0>(dc_state.uvw_bvalue_offset_acc) >> CONF_LG2_DC_CAL_TIMES),
                    int32_t(std::get<1>(dc_state.uvw_bvalue_offset_acc) >> CONF_LG2_DC_CAL_TIMES),
                    int32_t(std::get<2>(dc_state.uvw_bvalue_offset_acc) >> CONF_LG2_DC_CAL_TIMES)
                };


                const auto u_compare_res = compare_bvalue(dc_state.uvw_bvalue_offset[0]);
                const auto v_compare_res = compare_bvalue(dc_state.uvw_bvalue_offset[1]);
                const auto w_compare_res = compare_bvalue(dc_state.uvw_bvalue_offset[2]);

                auto raise_midpoint_outofrange_exception_ifneeded = [&](std::partial_ordering compare_order){
                    if(compare_order != std::partial_ordering::equivalent){

                        //TODO add exception recover
                        sys::trip();
                        __builtin_abort();
                    }
                };

                raise_midpoint_outofrange_exception_ifneeded(u_compare_res);
                raise_midpoint_outofrange_exception_ifneeded(v_compare_res);
                raise_midpoint_outofrange_exception_ifneeded(w_compare_res);

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


            if(not do_sideshaft_calibrate){
                [[maybe_unused]] const auto now_secs = clock::seconds();

                auto & traj_state = state.traj_state;
                #if 1
                static constexpr auto demo_pattern = 
                    // DemoTrajPattern::Sine
                    // DemoTrajPattern::Stop
                    DemoTrajPattern::Stairs
                    // DemoTrajPattern::Miniwave
                    // DemoTrajPattern::Saw
                    // DemoTrajPattern::Triangle
                ;

                traj_state = calc_demo_traj(now_secs, demo_pattern);
                #else
                static uint32_t tick = 0;
                tick++;
                if(tick >= CONF_FOC_FREQ * 10) tick = 0;

                static constexpr auto RBTRIP_PARAS = motioner::RoundtripParaments{
                    .fs = CONF_FOC_FREQ,
                    // .revs_per_direction = 2, 
                    .uniform_ticks = 2 * SIDESHAFT_CALIBRATE_STEPS_PER_REV,
                    .ticks_per_rev = SIDESHAFT_CALIBRATE_STEPS_PER_REV,
                    .x1_initial = iiq32(-7), 
                };

                static constexpr auto ROUNDTRIP_CALC = motioner::RoundtripTrajGenerator::from(RBTRIP_PARAS);
                auto samp_point = ROUNDTRIP_CALC.sample_tick(tick);


                traj_state.x1 = samp_point.x1;
                traj_state.x2 = samp_point.x2;
                traj_state.x3 = samp_point.x3;

                #endif


                process_traj_preshape(state, fn_switches, traj_state.x1, traj_state.x2, traj_state.x3);

                auto & hp_traj_state = state.traj_smooth_state;
                process_traj_shape(state, fn_switches, hp_traj_state.x1, hp_traj_state.x2, hp_traj_state.x3);

                const auto curve_x1 = state.curve_state.x1;
                const auto curve_x2 = state.curve_state.x2;
                const auto curve_x3 = state.curve_state.x3;
                process_mechanical_loop(state, fn_switches, curve_x1, curve_x2, curve_x3);
            }


            if(do_sideshaft_calibrate){
                {

                    static constexpr size_t LG2_STEPS_CURRENT_RAMP = 14u;

                    static constexpr iq16 ELEC_X2 = iq16(CONF_FOC_FREQ * 1.0 / SIDESHAFT_CALIBRATE_STEPS_PER_REV);
                    [[maybe_unused]] static constexpr float SECONDS_PER_REV = double(SIDESHAFT_CALIBRATE_STEPS_PER_REV) / CONF_FOC_FREQ;
                    [[maybe_unused]] static constexpr float SECONDS_RAMP = double(1 << LG2_STEPS_CURRENT_RAMP) / CONF_FOC_FREQ;

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
                            if(next_count_value >= SIDESHAFT_CALIBRATE_STEPS_PER_REV * CALIBRATE_REVS_PER_DIRECTION){
                                next_stage = Stage::Backward;
                                next_count_value = 0;
                            }

                            const auto now_steps = now_count_value;

                            const auto cmd_mech_turns = ec_state.cmd_mech_turns = steps_to_turns(now_steps);
                            
                            const auto mech_eps = warp_encoder_err(cmd_mech_turns, encoder_position_raw);

                            const auto frac = (encoder_position_raw * SIDESHAFT_EPS_TABLE_LENGTH);
                            const auto index = intrinsics::mul32hu(SIDESHAFT_EPS_TABLE_LENGTH, encoder_position_raw.to_bits());

                            const auto table_index = warp_index(index + (frac >= 0.5_uq32), SIDESHAFT_EPS_TABLE_LENGTH);

                            if(now_count_value % (SIDESHAFT_CALIBRATE_STEPS_PER_SAMPLE) == 0){
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

                            if(next_count_value >= SIDESHAFT_CALIBRATE_STEPS_PER_REV * CALIBRATE_REVS_PER_DIRECTION){
                            // if(next_count_value >= 0){
                                next_stage = Stage::Complete;
                                next_count_value = 0;
                            }

                            const auto now_steps = now_count_value;

                            const auto cmd_mech_turns = ec_state.cmd_mech_turns = steps_to_turns(now_steps);
                            
                            const auto table_eps = calc_encoder_eps(ec_state.eps_table.data(), encoder_position_raw);
                            [[maybe_unused]] const auto enc_mech_turns_after = encoder_position_raw + table_eps;

                            ec_state.debug.mech_eps_before = warp_encoder_err(cmd_mech_turns, encoder_position_raw);
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
    };

    auto isr_foc = [&]{
        auto & state = all_state_;

        timming_watch_pin_.set_high();
        state.isr_entry_tick = get_timer_tick();

        foc_loop();

        state.isr_exit_tick = get_timer_tick();
        timming_watch_pin_.set_low();
    };

    hal::timer1.register_nvic<hal::TimerIT::CC4>(hal::NvicPriorityCode::highest(),  EN);
    hal::timer1.enable_interrupt<hal::TimerIT::CC4>(EN);
    hal::timer1.set_isr_callback([&](const hal::TimerEvent & event){
        switch(event){
            case hal::TimerEvent::CC4:{
                isr_foc();
                break;
            }
            default:
                break;
        }
    });

    hal::timer1.start();

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
                all_state_.traj_state.x1 = make_iiq32(x);
            }),
            
            script::make_function(StringView("sce"), [&](const bool en){
                fn_switches_.sideshaft_compensate_en = en;
            }),

            script::make_function(StringView("ehse"), [&](const bool en){
                fn_switches_.encoder_harmonic_suppression_en = en;
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
        const uint32_t temp_bvalue = static_cast<uint32_t>(ADC1->IDATAR1);

        state.temperature_state.die().celsius = lpf_10hz(
            state.temperature_state.die().celsius, TEMP_TRIMER.parse_bvalue_u12(temp_bvalue));

        #if 0
        // {
            const auto offset = state.hfi_state.spinhfi_bin0_real_response;
            const auto amp = math::mag(state.hfi_state.spinhfi_bin2_real_response, hfi_state.spinhfi_bin2_imag_response) * 2;
            static constexpr auto factor = (int)(iq12(CONF_FOC_FREQ) / iq12(CONF_HFI_MODU_DEPTH_LIMIT * BUSBAR_VOLT));
            [[maybe_unused]] const auto lq_est_mh = iq20(1000.0 / factor) / (offset - amp);
            [[maybe_unused]] const auto ld_est_mh = iq20(1000.0 / factor) / (offset + amp);
        // }

        // {
            [[maybe_unused]] const auto mag_volt = math::mag(state.dq_volt_integral[0], state.dq_volt_integral[1]);
            [[maybe_unused]] const auto inv_mag_curr = math::inv_mag(state.alphabeta_curr_raw[0], state.alphabeta_curr_raw[1]);
        // }
        #endif


        if(false)DEBUG_PRINTLN(
            math::fixed_downcast<16>(state.traj_state.x1),
            math::fixed_downcast<16>(state.curve_state.x1),
            math::fixed_downcast<16>(state.encoder_ltd_state.x1),
            // iq16::from_bits(int32_t(differential_int64(state.differ, state.curve_state.x1.to_bits()) >> 6)),
            math::fixed_downcast<16>(state.curve_state.x1) - math::fixed_downcast<16>(state.encoder_ltd_state.x1),
            state.encoder_ltd_state.x2,
            tmrticks_to_us(state.isr_exit_tick) - tmrticks_to_us(state.isr_entry_tick)
            // tmrticks_to_us(state.encoder_get_done_tick) - tmrticks_to_us(state.isr_entry_tick)
        );

        // auto & ec_state = state.encoder_calibrate_state;

        [[maybe_unused]] auto encode_curr_eps = [](const iq20 curr_err) -> int16_t{
            // int16_t bits = (curr_err.to_bits() >> 8) & ((1u << 12) - 1);
            static constexpr size_t LG2_RESOLUTION = 12;
            static constexpr size_t Q_NUM = 20;
            static constexpr size_t LEFT_SHIFTS = LG2_RESOLUTION + (32u - Q_NUM); 
            int32_t bits = intrinsics::mul32hsu(curr_err.to_bits(), 1u << LEFT_SHIFTS);
            return int16_t(bits);
        };

        #if 0
        if(false) DEBUG_PRINTLN(
            profiler_.try_nth_record(0),
            profiler_.try_nth_record(1)
        );
        #endif

        enum class HomeMethod:uint8_t{
            Initial,
            // CeilInitial,
            // FloorInitial,
            NearestZerosign,
            CeilZerosign,
            FloorZerosign,
        };

        [[maybe_unused]] const auto encoder_rel_p64 = state.encoder_rel_position64;
        [[maybe_unused]] const auto encoder_initial_p32 = state.encoder_initial_position;
        [[maybe_unused]] auto calc_home_position = [&](HomeMethod method) -> iiq32{

            switch(method){
                case HomeMethod::Initial:{
                    return 0;
                    break;
                }
                // case HomeMethod::CeilInitial:{

                //     break;
                // }
                // case HomeMethod::FloorInitial:{
                //     break;
                // }
                case HomeMethod::NearestZerosign:{
                    const bool need_up_round = bool(encoder_initial_p32.to_bits() >> 31);
                    auto p64 = iiq32::from_bits(-int64_t(encoder_initial_p32.to_bits()));
                    return iiq32_add_revs(p64, int32_t(need_up_round));
                    break;
                }
                case HomeMethod::CeilZerosign:{
                    return iiq32_add_revs(iiq32::from_bits(-int64_t(encoder_initial_p32.to_bits())),1);
                    break;
                }
                case HomeMethod::FloorZerosign:{
                    return iiq32::from_bits(-int64_t(encoder_initial_p32.to_bits()));
                    break;
                }
            }
            return 0;
        };

        if(true)DEBUG_PRINTLN(

            math::fixed_downcast<16>(state.traj_state.x1),
            // state.traj_state.x1,
            math::fixed_downcast<16>(state.traj_smooth_state.x1),
            math::fixed_downcast<16>(state.curve_state.x1),
            math::fixed_downcast<16>(state.encoder_ltd_state.x1),
            // ,
            // state.encoder_ltd_state.x2,
            // math::fixed_downcast<16>(state.encoder_pll_state.x1),
            // math::fixed_downcast<16>(state.encoder_rel_position64),
            // state.encoder_pll_state.x2,

            // ADC_LSB_PER_CURRENT_AMPS * state.uvw_curr_ref.u + state.dc_calibrate_state.uvw_bvalue_offset[0],
            // state.uvw_adc_bvalue,
            // state.uvw_curr_raw,
            // state.uvw_curr_ref,
            state.temperature_state.die().celsius,
            // math::fixed_downcast<16>(state.encoder_abs_position64),
            // math::fixed_downcast<16>(state.encoder_rel_position64 - state.curve_state.x1) * 360,


            // state.traj_smooth_state.x2,
            // state.curve_state.x2,
            // state.sensed_elec_angle.to_turns(),
            // state.sensed_elec_speed,
            // state.torque_curr_cmd,
            // math::fixed_downcast<16>(calc_home_position(HomeMethod::NearestZerosign)),
            // math::fixed_downcast<16>(calc_home_position(HomeMethod::CeilZerosign)),
            // math::fixed_downcast<16>(calc_home_position(HomeMethod::FloorZerosign)),
            // state.peac_state.hat_b1,
            // state.peac_state.hat_b2,
            // state.peac_state.debug.e,
            // state.peac_state.harm * 100,
            // state.encoder_ltd_state.x2,
            // state.pi_ref_x2,

            state.torque_curr_cmd,
            state.dq_curr_ref,
            // state.torque_curr_veryslowlp,
            // iq20::from(float(math::bf16(float(state.torque_curr_veryslowlp)))),
            // encode_curr_eps(state.torque_curr_veryslowlp),
            // state.retrack_count,
            // state.busbar_curr_lp,
            // state.debug.traj_e1,
            // math::fixed_downcast<16>(state.debug.traj_x1),
            // math::fixed_downcast<16>(state.debug.curve_x1),
            // state.peac_state.debug.harm_turns,
            // state.peac_state.harm_c,
            // state.temperature_state.die().celsius,
            // tmrticks_to_us(state.isr_entry_tick),
            // tmrticks_to_us(state.encoder_get_done_tick),
            tmrticks_to_us(state.isr_exit_tick)
        );

        poll_led_blink();
        // toggle_red_led();
        // repl_service_poller();
    }

}

