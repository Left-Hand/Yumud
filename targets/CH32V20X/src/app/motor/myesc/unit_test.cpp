#include "dsp_lpf.hpp"
#include "dsp_vec.hpp"

using namespace ymd;
using namespace ymd::dsp;

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

}

static_assert(dot2v2(1_iq20, 2_iq20, 3_iq20, 4_iq20) == 14_iq20);
static_assert(dot2v2(1_iq16, 2_iq16, 3_iq16, 4_iq16) == 14_iq16);
static_assert(cross2v2(1_iq20, 2_iq20, 3_iq20, 4_iq20) == -2_iq20);
static_assert(cross2v2(1_iq16, 2_iq16, 3_iq16, 4_iq16) == -2_iq16);