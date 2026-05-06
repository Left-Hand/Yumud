#pragma once


#include "core/math/fixed/fxmath.hpp"
#include "core/utils/Result.hpp"
#include "core/string/view/string_view.hpp"
#include "primitive/arithmetic/angular.hpp"

// https://blog.51cto.com/u_14344/14422008
// https://zhuanlan.zhihu.com/p/1941084187869282361
namespace ymd::dsp{


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


static constexpr Result<math::fixed<32, uint32_t>, StringView> calc_lpf_alpha_uq32(uint32_t fs, uint32_t fc){
    constexpr size_t SHIFT_BITS = 9;
    constexpr size_t MAX_FREQ = (1u << (32u - SHIFT_BITS)) / 8;  // div 8 for margin
    
    // 参数检查
    if(fs == 0) return Err(StringView("fs cannot be zero"));
    if(fs >= MAX_FREQ) return Err(StringView("fs overflow")); 
    if(fc >= MAX_FREQ) return Err(StringView("fc overflow"));
    if(fc * 2 >= fs) return Err(StringView("nyquist failed"));

    // 使用安全的乘法，防止中间溢出
    const uint64_t pow2_32_SHIFT = static_cast<uint64_t>(1) << (32 + SHIFT_BITS);
    const uint64_t pow2_SHIFT = static_cast<uint64_t>(1) << SHIFT_BITS;
    
    // 计算分子：fs * 2^(32+SHIFT_BITS)
    const uint64_t num = static_cast<uint64_t>(fs) * pow2_32_SHIFT;
    
    // 计算分母：fs * 2^SHIFT_BITS + fc * TAU * 2^SHIFT_BITS
    const uint64_t fs_term = static_cast<uint64_t>(fs) * pow2_SHIFT;
    
    // 确保 TAU 有足够的精度和适当的缩放
    constexpr uint64_t TAU_SCALED = static_cast<uint64_t>(TAU * (1ull << SHIFT_BITS) + 0.5);
    const uint64_t fc_term = static_cast<uint64_t>(fc) * TAU_SCALED;
    
    const uint64_t den = fs_term + fc_term;
    
    // 防止除零
    if(den == 0) return Err(StringView("denominator is zero"));
    
    return Ok(math::fixed<32, uint32_t>::from_bits(~static_cast<uint32_t>(num / den)));
}



static constexpr Angular<uq32> calc_lpf_phaseshift_uq32(iq16 fc, iq16 f) {
    const auto turns = atan2pu(static_cast<iq16>(f), static_cast<iq16>(fc));
    return Angular<uq32>::from_turns(math::pu_to_uq32(turns));
}


//y[n] = alpha * x[n] + beta * y[n-1]
template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_1o(
    const math::fixed<Q, int32_t> x_state, 
    const math::fixed<Q, int32_t> x_new, 
    const uq32 alpha
){
    const uq32 beta = uq32::from_bits(~alpha.to_bits());
    using acc_t = std::conditional_t<std::is_signed_v<int32_t>, int64_t, uint64_t>;
    return math::fixed<Q, int32_t>::from_bits(
        int32_t((static_cast<acc_t>(x_new.to_bits()) * alpha.to_bits()) >> 32)
        + int32_t((static_cast<acc_t>(x_state.to_bits()) * beta.to_bits()) >> 32)
    );
}

struct Lpf1o{
    struct Config{
        uint32_t fs;
        uint32_t fc;

        constexpr Result<Lpf1o, StringView> try_into_precomputed() const noexcept {
            // const ((fs) << (9 + 32u)) / ((fs << 9) + static_cast<uint32_t>(TAU * (1u << 9)) * fc);
            const uq16 num = static_cast<uq16>(fs);
            const uq16 den = static_cast<uq16>(fs) + static_cast<uq16>(TAU) * static_cast<uq16>(fc);
            return Ok(Lpf1o{
                .alpha = static_cast<uq32>(num / den)
            });
        }
    };

    math::fixed<32, uint32_t> alpha;

    template<size_t Q>
    void iterate(const math::fixed<Q, int32_t> state, const math::fixed<Q, int32_t> input){
        lpf_1o(state, input, alpha);
    }
};





}
