#include "dsp_lpf.hpp"
#include "dsp_vec.hpp"

using namespace ymd;
using namespace ymd::dsp;


template<typename Fn1, typename Fn2, typename ... Args>
static consteval bool is_result_nearly_equal(Fn1 && fn1, Fn2 && fn2, const long double eps, Args && ... args){
    const auto res1 = fn1(std::forward<Args>(args)...);
    const auto res2 = fn2(std::forward<Args>(args)...);
    return std::abs(static_cast<long double>(res1.unwrap()) - static_cast<long double>(res2.unwrap())) < eps;
}

template<std::floating_point T>
static constexpr T abs_err_percentages(const T dst, const T src){
    const auto abs_err = math::abs(dst - src);
    return static_cast<T>(abs_err * 100 / src);
}

static_assert(abs_err_percentages(
    double(TAU_SCALE_NUM) / TAU_SCALE_DEN, double(TAU)) < 3e-4);
static_assert(abs_err_percentages(
    double(INV_TAU_SCALE_NUM) / INV_TAU_SCALE_DEN, double(1.0 / TAU)) < 2e-2);



namespace {
// static_assert(calc_lpf_alpha_uq32(16000, 10).unwrap())
static_assert(is_result_nearly_equal(
    [](uint32_t fs, uint32_t fc){ return calc_lpf_alpha_f32(fs, fc); },
    [](uint32_t fs, uint32_t fc){ return calc_lpf_alpha_uq32(fs, fc); },
    0.01,
    16000, 10
));

static_assert(is_result_nearly_equal(
    [](uint32_t fs, uint32_t fc){ return calc_lpf_alpha_f32(fs, fc); },
    [](uint32_t fs, uint32_t fc){ return calc_lpf_alpha_uq32(fs, fc); },
    0.01,
    64000, 1400
));

[[maybe_unused]]static void test_phase_shift(){
    // static constexpr size_t FS = 8000;
    static constexpr size_t FC = 100;
    // static constexpr auto alpha = calc_lpf_alpha_f32(FS, FC).unwrap();
    static constexpr auto phase_shift_f32 = calc_lpf_phaseshift_f32(FC, FC).to_turns();
    static_assert(phase_shift_f32 == 0.125);
}

static_assert(math::abs((float)(calc_lpf_alpha_uq32(32000, 4000).unwrap()) - 0.439900846488) < 1e-6);
static_assert(math::abs((float)(calc_lpf_alpha_uq32(32000, 1000).unwrap()) - 0.164123890339) < 1e-6);
static_assert(math::abs((float)(calc_lpf_alpha_uq32(32000, 600).unwrap()) - 0.105393361613) < 1e-6);
static_assert(math::abs((float)(calc_lpf_alpha_uq32(32000, 200).unwrap()) - 0.0377860533257) < 1e-6);
static_assert(math::abs((float)(calc_lpf_alpha_uq32(32000, 4).unwrap()) - 0.000784781797215) < 1e-6);
static_assert(math::abs((float)(calc_lpf_alpha_uq32(32000, 1).unwrap()) - 0.000196310995276) < 1e-6);


}

// static_assert(dsp::lpf_1o(1_iq20, 2_iq20, 0.5_uq32).to_bits() == 1.5_iq20);
// static_assert(dsp::lpf_1o(1_iq20, 2_iq20, 0.125_uq32) == 1.125_iq20);


static_assert(dot2v2(1_iq20, 2_iq20, 3_iq20, 4_iq20) == 14_iq20);
static_assert(dot2v2(1_iq16, 2_iq16, 3_iq16, 4_iq16) == 14_iq16);
static_assert(cross2v2(1_iq20, 2_iq20, 3_iq20, 4_iq20) == -2_iq20);
static_assert(cross2v2(1_iq16, 2_iq16, 3_iq16, 4_iq16) == -2_iq16);






static_assert(std::abs((float)(heightleg(5.0_iq20, 3.0_iq20)) - 4.0f) < 1e-4);
static_assert(std::abs((float)(heightleg(5.0_iq20, 4.0_iq20)) - 3.0f) < 1e-4);

static_assert(std::abs((float)std::get<0>(resat_unit_circle(0.2_iq20, 0.0_iq20)) - 1.0f) < 1e-4);

static_assert(std::abs((float)std::get<0>(resat_unit_circle(1.0_iq20, 1.73205080757_iq20)) - 0.5f) < 1e-4);
static_assert(std::abs((float)std::get<1>(resat_unit_circle(1.0_iq20, 1.73205080757_iq20)) - 0.866025403784f) < 1e-4);

static_assert(std::abs((float)std::get<0>(resat_unit_circle(0.010_iq20, 0.0173205080757_iq20)) - 0.5f) < 1e-4);
static_assert(std::abs((float)std::get<1>(resat_unit_circle(0.010_iq20, 0.0173205080757_iq20)) - 0.866025403784f) < 1e-4);