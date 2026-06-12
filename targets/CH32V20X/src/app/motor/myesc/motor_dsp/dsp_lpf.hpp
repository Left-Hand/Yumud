#pragma once


#include "core/math/fixed/fxmath.hpp"
#include "core/utils/Result.hpp"
#include "core/string/view/string_view.hpp"
#include "primitive/arithmetic/angular.hpp"

// https://blog.51cto.com/u_14344/14422008
// https://zhuanlan.zhihu.com/p/1941084187869282361
namespace ymd::dsp{

static constexpr size_t TAU_SCALE_SHIFT = 9;
static constexpr size_t TAU_SCALE_NUM = 3217;
static constexpr size_t TAU_SCALE_DEN = 1 << TAU_SCALE_SHIFT;

static constexpr size_t INV_TAU_SCALE_SHIFT = 10;
static constexpr size_t INV_TAU_SCALE_NUM = 163;
static constexpr size_t INV_TAU_SCALE_DEN = 1 << INV_TAU_SCALE_SHIFT;

static consteval int64_t pow2_to_i64(const long double x, size_t n){
    const uint64_t i = uint64_t(1) << n;
    return x * i;
}

static constexpr Result<float, StringView> calc_lpf_alpha_f32(uint32_t fs, uint32_t fc){
    if(fs == 0) return Err(StringView("fs cannot be zero"));
    if(fc * 2 >= fs) return Err(StringView("nyquist failed"));

    const float wc = (static_cast<float>(TAU) * fc);
    return Ok(static_cast<float>(
        wc / (fs + wc))
    );
}

static constexpr Angular<float> calc_lpf_phaseshift_f32(uint32_t fc, uint32_t f) {
    return Angular<float>::from_atan2(f, fc);
}


static constexpr Result<uq32, StringView> calc_lpf_alpha_uq32(uint32_t fs, uint32_t fc){
    // 计算 alpha = 2π·fc / (fs + 2π·fc)
    
    if(fs == 0) return Err(StringView("fs cannot be zero"));
    if(fc * 2 >= fs) return Err(StringView("nyquist failed"));

    const uint32_t num = fc * TAU_SCALE_NUM;     // 2π·fc
    const uint32_t den = fs * TAU_SCALE_DEN + fc * TAU_SCALE_NUM;  // fs + 2π·fc
    
    const uq32 alpha = uq32::from_bits(num) / uq32::from_bits(den);
    return Ok(alpha);
}





template<size_t Q>
__always_inline static constexpr math::fixed<Q, int32_t> lpf_1o(
    const math::fixed<Q, int32_t> y_prev,  // y[n-1]
    const math::fixed<Q, int32_t> x,       // x[n]
    const uq32 alpha
){
    #if 1
    return y_prev + (x - y_prev) * alpha;

    #else
    const uq32 beta = uq32::from_bits(~alpha.to_bits());  // 1 - alpha
    
    return math::fixed<Q, int32_t>::from_bits(
        intrinsics::mul32hsu(x.to_bits(), alpha.to_bits()) + 
        intrinsics::mul32hsu(y_prev.to_bits(), beta.to_bits())
    );
    #endif
}

template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_1o_inplace(
    math::fixed<Q, int32_t> & y_state, 
    const math::fixed<Q, int32_t> x, 
    const uq32 alpha
){
    y_state = lpf_1o(y_state, x, alpha);
}

static constexpr Angular<uq32> calc_lpf_phaseshift_uq32(iq16 fc, iq16 f) {
    const auto turns = atan2pu(static_cast<iq16>(f), static_cast<iq16>(fc));
    return Angular<uq32>::from_turns(math::pu_to_uq32(turns));
}

#if 0
struct Lpf1o{
    struct Config{
        uint32_t fs;
        uint32_t fc;

        constexpr Result<Lpf1o, StringView> try_into_precomputed() const noexcept {
            return Ok(Lpf1o{
                .alpha = calc_lpf_alpha_uq32(fs, fc).map([auto & x]);
            });
        }
    };

    uq32 alpha;

    template<size_t Q>
    void iterate(math::fixed<Q, int32_t> & state, const math::fixed<Q, int32_t> input){
        state = lpf_1o(state, input, alpha);
    }
};
#endif





}
