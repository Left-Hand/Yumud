#include "iqmath.hpp"
#include <cmath>

using namespace ymd;
using namespace ymd::iqmath;
using namespace ymd::math;
using namespace ymd::literals;



// 添加更多测试
static_assert(iqmath::details::_IQFtoN<16>(0.0f) == 0);
static_assert(iqmath::details::_IQFtoN<16>(-0.0f) == 0);
static_assert(iqmath::details::_IQFtoN<16>(1.0f) == 65536);  // Q16: 1.0 = 65536
static_assert(iqmath::details::_IQFtoN<16>(-1.0f) == -65536);
static_assert(iqmath::details::_IQFtoN<16>(0.5f) == 32768);
static_assert(iqmath::details::_IQFtoN<16>(-0.5f) == -32768);
static_assert(iqmath::details::_IQFtoN<16>(0.25f) == 16384);
static_assert(iqmath::details::_IQFtoN<16>(1.5f) == 98304);  // 1.5 * 65536 = 98304

// 测试非常小的数
static_assert(iqmath::details::_IQFtoN<16>(0.0001f) == 6);  // 近似值
static_assert(iqmath::details::_IQFtoN<16>(-0.0001f) == -6);

// 测试Q不同值的情况
static_assert(iqmath::details::_IQFtoN<8>(1.0f) == 256);    // Q8: 1.0 = 256
static_assert(iqmath::details::_IQFtoN<8>(0.5f) == 128);
static_assert(iqmath::details::_IQFtoN<24>(1.0f) == 16777216);  // Q24: 1.0 = 16777216

static_assert(iqmath::details::_IQFtoN<16>(-32768.0f / 65536.0f) == -32768);


static_assert((iq16(4) >> 1) == iq16(2));
static_assert((iq16::from_rcp(4)) == iq16(0.25));
static_assert((uq16::from_rcp(4u)) == uq16(0.25));
static_assert((uuq32::from_rcp(32768u) * 32768u).to_bits() == uuq32(1).to_bits());
static_assert((iq16(1) + iq16(2)) == iq16(3));
static_assert((iq16(2) + iq16(1)) == iq16(3));
static_assert((iq16(2) * iq16(1)) == iq16(2));

static_assert(iq16(1919) / 2  == iq16(1919.0/2));
static_assert(iq16(1919) / 2u  == iq16(1919.0/2));
static_assert(iq16(-1919) / 2  == iq16(-1919.0/2));
static_assert((iq16(-1919) / 2u).to_bits()  == iq16(-1919.0/2).to_bits());

static_assert(1919 / iq16(2)  == iq16(1919.0/2));
static_assert(1919 / uq16(2)  == iq16(1919.0/2));
static_assert(-1919 / iq16(2)  == iq16(-1919.0/2));

static_assert((iq16(19) / iq16(2)).to_bits() == iq16(19/2.0).to_bits());
constexpr auto _19BY2_BITS_1 = iq16(19/2.0).to_bits();
constexpr auto _19BY2_BITS_2 = (iq16(19) / 2).to_bits();
static_assert((iq16(19) / 2).to_bits() == iq16(19/2.0).to_bits());
static_assert((iq16(-19) / 2).to_bits() == iq16(-19/2.0).to_bits());

static_assert((iq16(4) / 5).to_bits() == iq16(0.8).to_bits());
static_assert((iq16(4) / iq16(5)).to_bits() == iq16(0.8).to_bits());

static_assert(sqrt(iq16(4)) == iq16(2));
static_assert(sqrt(iq16(16)) == iq16(4));
static_assert(sqrt(iq16(64)) == iq16(8));
static_assert(sqrt(iq16(36)) == iq16(6));
static_assert(sqrt(iq16(16)) == iq16(4));

static_assert(sqrt(uq16(4)) == uq16(2));
static_assert(sqrt(uq16(16)) == uq16(4));

static_assert(sqrt(iq26(16)) == iq26(4));

static_assert(inv_sqrt(iq16(16)) == iq16(0.25));
static_assert(inv_sqrt(iq16(25)) == iq16(0.2));

static_assert(inv_sqrt(uq16(16)) == uq16(0.25));
static_assert(inv_sqrt(uq16(25)) == uq16(0.2));

static_assert(mag(iq16(3), iq16(4)) == iq16(5));
static_assert(inv_mag(iq16(3), iq16(4)).to_bits() == iq16(0.2).to_bits());
static_assert(inv_mag(iq16(12), iq16(5)).to_bits() == iq16(1.0/13).to_bits());

static_assert(sinpu(iq16(0)) == 0);
static_assert(sinpu(iq16(0.25)) == std::numeric_limits<iq31>::max());
static_assert(sinpu(iq16(0.5)) == iq16(0));
static_assert(sinpu(iq16(1.0)) == iq16(0));

static_assert(sinpu(iq10(0)) == iq10(0));
static_assert(sinpu(iq10(0.25)) == std::numeric_limits<iq31>::max());
static_assert(sinpu(iq10(0.5)) == iq10(0));
static_assert(sinpu(iq10(1.0)) == iq10(0));

static_assert(sinpu(iq30(0)) == iq30(0));
static_assert(sinpu(iq30(0.25)) == std::numeric_limits<iq31>::max());
static_assert(sinpu(iq30(0.5)) == iq30(0));
static_assert(sinpu(iq30(1.0)) == iq30(0));

static_assert(frac(iq16(0.25)) == iq16(0.25));
static_assert(frac(iq16(1.7)) == iq16(0.7));
static_assert(frac(uq16(1.7)) == uq16(0.7));


static_assert(atan2pu(iq16(1), iq16(1)) == iq16(0.125));
static_assert(atan2pu(iq16(1), iq16(-1)) == iq16(0.125 + 0.25));
static_assert(atan2pu(iq16(-1), iq16(-1)).to_bits() == iq16(0.125 - 0.5).to_bits());
static_assert(atan2pu(iq16(-1), iq16(1)).to_bits() == iq16(-0.125).to_bits());

static_assert(atan2pu(iq16(0), iq16(1)) == iq16(0));
static_assert(atan2pu(iq16(0), iq16(-1)).to_bits() == iq16(0.5).to_bits());

static_assert(atan2pu(iq16(1), iq16(0)) == iq16(0.25));
static_assert(atan2pu(iq16(-1), iq16(0)).to_bits() == iq16(-0.25).to_bits());

static_assert(round_int(iq16(1145.14)) == iq16(1145));
static_assert(round_int(iq16(1919.810)) == iq16(1920));

static_assert(ceil_int(iq16(1145.14)) == iq16(1146));
static_assert(ceil_int(iq16(1919.810)) == iq16(1920));

static_assert(floor_int(iq16(1145.14)) == iq16(1145));
static_assert(floor_int(iq16(1919.810)) == iq16(1919));

static_assert(round_int(iq16(-1145.14)) == iq16(-1145));
static_assert(round_int(iq16(-1919.810)) == iq16(-1920));

static_assert(ceil_int(iq16(-1145.14)) == iq16(-1145));
static_assert(ceil_int(iq16(-1919.810)) == iq16(-1919));

static_assert(floor_int(iq16(-1145.14)) == iq16(-1146));
static_assert(floor_int(iq16(-1919.810)) == iq16(-1920));

static_assert(signbit(iq16(0)) == 0);
static_assert(signbit(iq16(167)) == 0);
static_assert(signbit(iq16(-167)) == 1);
static_assert(signbit(iq31(0.5)) == 0);
static_assert(signbit(iq31(-0.5)) == 1);

static_assert(square(iq16(2)) = 4);
static_assert(square(uq29(2)) = 4);
static_assert(square(iq29(-2)) = 4);
static_assert(square(uq16(2)) = 4);
static_assert(square(uq16(100)) = 10000);


static_assert(abs(iiq32(2)) == 2);
static_assert(abs(iiq32(0)) == 0);
static_assert(iq31(-1) == iq16(-1));
static_assert(iq31(0) == iq16(0));
static_assert(iq31(-1) == iq24(-1));
static_assert(iq24(1) == iq16(1));
static_assert(fixed_t<15, int16_t>(0.5).to_bits() == iq16(0.25).to_bits());
static_assert(iq16::from_bits(fixed_t<15, int16_t>(0.25).to_bits()).to_bits() == iq15(0.25).to_bits());
static_assert(iq16::from_bits(fixed_t<15, int16_t>(-0.25).to_bits()).to_bits() == iq15(-0.25).to_bits());
static_assert(std::is_same_v<tmp::extended_mul_underlying_t<int32_t, int16_t>, int64_t>);
static_assert(std::is_same_v<tmp::extended_mul_underlying_t<int16_t, int16_t>, int32_t>);



// static_assert(pu_to_uq32(-1_iq16) == 0);
static_assert(pu_to_uq32(-0.75_iq16) == 0.25_uq32);
static_assert(pu_to_uq32(-0.75_iq10) == 0.25_uq32);
static_assert(pu_to_uq32(-0.75_iq31) == 0.25_uq32);
static_assert(pu_to_uq32(0.5_uq32) == 0.5_uq32);


static_assert(fabs(0.25 - double(rad_to_uq32(uq16((2 * M_PI) * 0.25)))) < 3E-5);
static_assert(fabs(0.75 - double(rad_to_uq32(uq16((2 * M_PI) * 1000.75)))) < 3E-5);
static_assert(fabs(0.25 - double(rad_to_uq32(iq16((2 * M_PI) * -100.75)))) < 3E-5);

static_assert(fabs(0.25 - double(rad_to_uq32(uq24((2 * M_PI) * 0.25)))) < 3E-5);
static_assert(fabs(0.75 - double(rad_to_uq32(uq24((2 * M_PI) * 10.75)))) < 3E-5);
static_assert(fabs(0.25 - double(rad_to_uq32(iq24((2 * M_PI) * -10.75)))) < 3E-5);

static_assert(fabs(0.25 - double(deg_to_uq32(uq16((360) * 0.25)))) < 3E-5);
static_assert(fabs(0.75 - double(deg_to_uq32(uq16((360) * 10.75)))) < 3E-5);
static_assert(fabs(0.25 - double(deg_to_uq32(iq16((360) * -10.75)))) < 3E-5);

static_assert(fabs(0.25 - double(deg_to_uq32(uq10((360) * 0.25)))) < 3E-5);
static_assert(fabs(0.75 - double(deg_to_uq32(uq10((360) * 100.75)))) < 3E-5);
static_assert(fabs(0.25 - double(deg_to_uq32(iq10((360) * -100.75)))) < 3E-5);

static_assert(double(std::get<0>(sincospu(0.5_uq32))) == 0);

static_assert(closer_to_zero(iiq32(2), iiq32(7)) == 2);
static_assert(closer_to_zero(iiq32(-2), iiq32(-3)) == iiq32(-2));

static_assert(closer_to_zero(iq16(2), iq16(0)) == 0);
static_assert(closer_to_zero(iq16(-2), iq16(2)) == 2);
