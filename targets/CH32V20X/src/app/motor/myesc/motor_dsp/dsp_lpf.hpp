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




__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr int32_t lerp_i32_uq32(
    const int32_t a, 
    const int32_t b, 
    const uq32 ratio
){
    return a + intrinsics::mul32hsu(b - a, ratio.to_bits());
}

__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr int32_t lerp_u32_uq32(
    const uint32_t a, 
    const uint32_t b, 
    const uq32 ratio
){
    return a + intrinsics::mul32hu(b - a, ratio.to_bits());
}


template<typename D>
requires(sizeof(D) <= 4)
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr D lerp_int_uq32(
    const D a, const D b, const uq32 ratio
){
    if constexpr(std::is_signed_v<D>){
        return lerp_i32_uq32(static_cast<int32_t>(a), static_cast<int32_t>(b), ratio);
    }else{
        return lerp_u32_uq32(static_cast<uint32_t>(a), static_cast<uint32_t>(b), ratio);
    }
}

template<size_t Q, typename D>
requires(sizeof(D) <= 4)
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, D> lerp_fixed_uq32(
    const math::fixed<Q, D> a, 
    const math::fixed<Q, D> b, 
    const uq32 ratio
){
    const auto y_bits = lerp_int_uq32(a.to_bits(), b.to_bits(), ratio);
    return math::fixed<Q, D>::from_bits(y_bits);
}

template<size_t Q, typename D>
requires(sizeof(D) <= 4)
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, D> lerp_fixed(
    const math::fixed<Q, D> a, 
    const math::fixed<Q, D> b, 
    const math::fixed<Q, D> ratio
){
    const auto ratio_uq32 = uq32::from_bits(ratio.to_bits() << (32u - Q));
    return lerp_fixed_uq32(a, b, ratio_uq32);
}

template<size_t Q>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, int32_t> lpf_1o(
    const math::fixed<Q, int32_t> y_prev,  // y[n-1]
    const math::fixed<Q, int32_t> x,       // x[n]
    const uq32 alpha
){
    return lerp_fixed_uq32(y_prev, x, alpha);
}

template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_1o_inplace(
    math::fixed<Q, int32_t> & y_state, 
    const math::fixed<Q, int32_t> x, 
    const uq32 alpha
){
    y_state = lpf_1o(y_state, x, alpha);
}

// 一阶HPF滤波函数（非原地）
template<size_t Q>
__always_inline static constexpr math::fixed<Q, int32_t> hpf_1o(
    const math::fixed<Q, int32_t> y_prev,  // y[n-1] 上一时刻输出
    const math::fixed<Q, int32_t> x,       // x[n] 当前输入
    const math::fixed<Q, int32_t> x_prev,  // x[n-1] 上一时刻输入
    const uq32 alpha
){
    // 一阶HPF差分方程：y[n] = alpha * (y[n-1] + x[n] - x[n-1])
    // 或者等价形式：y[n] = alpha * y[n-1] + alpha * (x[n] - x[n-1])
    // 更常用的形式：y[n] = alpha * (y[n-1] + x[n] - x[n-1])
    return (y_prev + x - x_prev) * alpha;
}

template<size_t Q>
__always_inline static constexpr math::fixed<Q, int32_t> hpf_1o_delta(
    const math::fixed<Q, int32_t> y_prev,  // y[n-1] 上一时刻输出
    const math::fixed<Q, int32_t> x_delta, 
    const uq32 alpha
){
    int32_t a = (y_prev + x_delta).to_bits();
    int32_t h32 = intrinsics::mul32hsu(a, alpha.to_bits());
    // uint32_t l32 = intrinsics::mul32(a, alpha.to_bits());
    // return math::fixed<Q, int32_t>::from_bits(h32 + bool(l32 >= 0x8000'0000 ));
    return math::fixed<Q, int32_t>::from_bits(h32 + 1);
}

#if 0
template<size_t Q>
__always_inline static constexpr math::fixed<Q, int32_t> hpf_1o_delta(
    const math::fixed<Q, int32_t> y_prev,      // y[n-1]
    const math::fixed<Q, int32_t> x_delta,   // x[n] - x[n-1]
    const uq32 alpha                           // 接近1的定点数，例如 0.9999
) {
    // 使用 int64_t 作为中间累加器，彻底防止数值溢出
    int64_t y_prev_raw = static_cast<int64_t>(y_prev.raw());      // 上一时刻输出
    int64_t alpha_raw = static_cast<int64_t>(alpha.raw());        // 系数

    // 计算差分（此处不要直接截断，用64位保存微小值）
    int64_t delta_x = x_delta.to_bits();

    // 正确的差分方程： y[n] = alpha * y[n-1] + alpha * (x[n] - x[n-1])
    // 为了防止 alpha 乘法溢出，先除以 Q 格式的基数（即 2^Q）
    // 注意：这里的 alpha 是 (0,1) 的小数，乘以 raw 值后再右移 Q 位
    int64_t term1 = (alpha_raw * y_prev_raw) >> 32;   // alpha * y[n-1]
    int64_t term2 = (alpha_raw * delta_x) >> 32;      // alpha * (x[n] - x[n-1])

    int64_t y_raw = term1 + term2;

    // --- 关键步骤：抗积分饱和（Anti-windup） ---
    // 钳位到 int32_t 范围内，防止溢出
    if (y_raw > INT32_MAX) y_raw = INT32_MAX;
    if (y_raw < INT32_MIN) y_raw = INT32_MIN;

    // 返回定点数
    return math::fixed<Q, int32_t>(static_cast<int32_t>(y_raw));
}
#endif

// 一阶HPF滤波函数（原地版本，只保存输出状态）
template<size_t Q>
static constexpr math::fixed<Q, int32_t> hpf_1o_inplace(
    math::fixed<Q, int32_t> & y_state,   // y[n-1] 输入上一时刻输出，输出更新为当前时刻输出
    const math::fixed<Q, int32_t> x,     // x[n] 当前输入
    const math::fixed<Q, int32_t> x_prev, // x[n-1] 上一时刻输入
    const uq32 alpha
){
    // 计算当前输出并更新状态
    y_state = hpf_1o(y_state, x, x_prev, alpha);
    return y_state;
}

static constexpr Angular<uq32> calc_lpf_phaseshift_uq32(iq16 fc, iq16 f) {
    const auto turns = atan2pu(static_cast<iq16>(f), static_cast<iq16>(fc));
    return Angular<uq32>::from_turns(math::pu_to_uq32(turns));
}



}
