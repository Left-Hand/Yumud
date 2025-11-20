#include "bf16.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{
OutputStream & operator << (OutputStream & os, const bf16 f_val){
    // uint16_t frac:7;
    // uint16_t exp:8;
    // uint16_t sign:1;
    // return os << float(1.0);
    // return os << fixed_t<16, int32_t>::from(float(f_val));
    return os << float(f_val);
    // return os << os.scoped("bf16")(
    //     os << os.field("frac")(f_val.raw.frac) 
    //     << os.field("exp")(f_val.raw.exp) 
    //     << os.field("sign")(f_val.raw.sign) << os.endl()
    // );
}

#if 0

// 测试 bf16 的位域是否正确
static_assert(sizeof(bf16) == 2, "bf16 must be 2 bytes");

// 测试 0.0 的表示
constexpr bf16 zero = bf16(0.0f);
static_assert(zero.sign() == 0, "sign(0.0) should be 0");
static_assert(zero.exp() == 0, "exp(0.0) should be 0");
static_assert(zero.frac() == 0, "frac(0.0) should be 0");

// 测试 -0.0 的表示
constexpr bf16 neg_zero = bf16(-0.0f);
static_assert(neg_zero.sign() == 1, "sign(-0.0) should be 1");
static_assert(neg_zero.exp() == 0, "exp(-0.0) should be 0");
static_assert(neg_zero.frac() == 0, "frac(-0.0) should be 0");
static_assert(neg_zero.to_u16() == 0);

// 测试 1.0 的表示
constexpr bf16 one = bf16(1.0f);
static_assert(one.sign() == 0, "sign(1.0) should be 0");
static_assert(one.exp() == 127, "exp(1.0) should be 127 (biased)");
static_assert(one.frac() == 0, "frac(1.0) should be 0");
static_assert(one.to_u16() == 0);

// 测试 -1.0 的表示
constexpr bf16 neg_one = bf16(-1.0f);
static_assert(neg_one.sign() == 1, "sign(-1.0) should be 1");
static_assert(neg_one.exp() == 127, "exp(-1.0) should be 127 (biased)");
static_assert(neg_one.frac() == 0, "frac(-1.0) should be 0");

// 测试最小正规数（~1.18e-38）
constexpr bf16 min_normal = bf16::from_u16(0x0080); // exp=1, frac=0
static_assert(min_normal.sign() == 0, "sign(min_normal) should be 0");
static_assert(min_normal.exp() == 1, "exp(min_normal) should be 1 (biased)");
static_assert(min_normal.frac() == 0, "frac(min_normal) should be 0");
static_assert(min_normal.to_u16() == 0x0080);

// 测试最大正规数（~3.39e38）
constexpr bf16 max_normal = bf16::from_u16(0x7F7F); // exp=254, frac=127
static_assert(max_normal.sign() == 0, "sign(max_normal) should be 0");
static_assert(max_normal.exp() == 254, "exp(max_normal) should be 254 (biased)");
static_assert(max_normal.frac() == 127, "frac(max_normal) should be 127");

// 测试 NaN（指数全1，尾数非0）
constexpr bf16 nan_bf16 = bf16::from_u16(0x7F80); // exp=255, frac=1
static_assert(nan_bf16.sign() == 0, "sign(NaN) should be 0");
static_assert(nan_bf16.exp() == 255, "exp(NaN) should be 255 (biased)");
static_assert(nan_bf16.frac() == 1, "frac(NaN) should be non-zero");

#endif 

}