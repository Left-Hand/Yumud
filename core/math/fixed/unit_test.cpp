#include "iqmath.hpp"
#include <cmath>

using namespace ymd;
using namespace ymd::iqmath;
using namespace ymd::literals;

namespace{
// 添加更多测试


static_assert(iq24(10.0) + 90 == 100);
static_assert(iq16(10.0) + 90 == 100);
static_assert(iq10(10.0) + 90 == 100);

[[maybe_unused]] void test_add(){
    auto add2 = [&](const iq16 a, const int8_t b) -> iq16 {return a + b;};
    // auto add2 = [&](const iq16 a, const int8_t b){return a + b;};
    static_assert((iq16(9)) == 9);
    static_assert((iq16(int8_t(9))) == 9);
    static_assert((iq16(10.0) + int8_t(9)) == 19);
    static_assert(add2(iq16(10.0), int8_t(90)) == 100);
}

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

static_assert(math::abs(iiq32(2)) == 2);
static_assert(math::abs(iiq32(0)) == 0);
static_assert(iq31(-1) == iq16(-1));
static_assert(iq31(0) == iq16(0));
static_assert(iq31(-1) == iq24(-1));
static_assert(iq24(1) == iq16(1));
static_assert(math::fixed<15, int16_t>(0.5).to_bits() == iq16(0.25).to_bits());
static_assert(iq16::from_bits(math::fixed<15, int16_t>(0.25).to_bits()).to_bits() == iq15(0.25).to_bits());
static_assert(iq16::from_bits(math::fixed<15, int16_t>(-0.25).to_bits()).to_bits() == iq15(-0.25).to_bits());
static_assert(std::is_same_v<tmp::extended_mul_underlying_t<int32_t, int16_t>, int64_t>);
static_assert(std::is_same_v<tmp::extended_mul_underlying_t<int16_t, int16_t>, int32_t>);

// static_assert(math::pu_to_uq32(-1_iq16) == 0);
static_assert(math::pu_to_uq32(-0.75_iq16) == 0.25_uq32);
static_assert(math::pu_to_uq32(-0.75_iq10) == 0.25_uq32);
static_assert(math::pu_to_uq32(-0.75_iq31) == 0.25_uq32);
static_assert(math::pu_to_uq32(0.5_uq32) == 0.5_uq32);

static_assert(-0.25_iq16 == iq32(-0.25));
static_assert(-0.25_iq10 == iq32(-0.25));
static_assert(-0.25_iq31 == iq32(-0.25));


static_assert(std::abs(0.25 - double(rad_to_uq32(uq16((2 * M_PI) * 0.25)))) < 3E-5);
static_assert(std::abs(0.75 - double(rad_to_uq32(uq16((2 * M_PI) * 1000.75)))) < 3E-5);
static_assert(std::abs(0.25 - double(rad_to_uq32(iq16((2 * M_PI) * -100.75)))) < 3E-5);

static_assert(std::abs(0.25 - double(rad_to_uq32(uq24((2 * M_PI) * 0.25)))) < 3E-5);
static_assert(std::abs(0.75 - double(rad_to_uq32(uq24((2 * M_PI) * 10.75)))) < 3E-5);
static_assert(std::abs(0.25 - double(rad_to_uq32(iq24((2 * M_PI) * -10.75)))) < 3E-5);

static_assert(std::abs(0.25 - double(deg_to_uq32(uq16((360) * 0.25)))) < 3E-5);
static_assert(std::abs(0.75 - double(deg_to_uq32(uq16((360) * 10.75)))) < 3E-5);
static_assert(std::abs(0.25 - double(deg_to_uq32(iq16((360) * -10.75)))) < 3E-5);

static_assert(std::abs(0.25 - double(deg_to_uq32(uq10((360) * 0.25)))) < 3E-5);
static_assert(std::abs(0.75 - double(deg_to_uq32(uq10((360) * 100.75)))) < 3E-5);
static_assert(std::abs(0.25 - double(deg_to_uq32(iq10((360) * -100.75)))) < 3E-5);

static_assert(double(std::get<0>(sincospu(0.5_uq32))) == 0);

static_assert(math::closer_to_zero(iiq32(2), iiq32(7)) == 2);
static_assert(math::closer_to_zero(iiq32(-2), iiq32(-3)) == iiq32(-2));

static_assert(math::closer_to_zero(iq16(2), iq16(0)) == 0);
static_assert(math::closer_to_zero(iq16(-2), iq16(2)) == 2);


static_assert(math::frac(iq16(0.25)) == iq16(0.25));
static_assert(math::frac(iq16(1.7)) == iq16(0.7));
static_assert(math::frac(uq16(1.7)) == uq16(0.7));



static_assert(math::round_int(iq16(1145.14)) == iq16(1145));
static_assert(math::round_int(iq16(1919.810)) == iq16(1920));

static_assert(math::round_int(iq16(-1145.14)) == iq16(-1145));
static_assert(math::round_int(iq16(-1919.810)) == iq16(-1920));

static_assert(math::ceil_int(iq16(1145.14)) == iq16(1146));
static_assert(math::ceil_int(iq16(1919.810)) == iq16(1920));

static_assert(math::ceil_int(iq16(-1145.14)) == iq16(-1145));
static_assert(math::ceil_int(iq16(-1919.810)) == iq16(-1919));

static_assert(math::floor_int(iq16(1145.14)) == iq16(1145));
static_assert(math::floor_int(iq16(1919.810)) == iq16(1919));

static_assert(math::floor_int(iq16(-1145.14)) == iq16(-1146));
static_assert(math::floor_int(iq16(-1919.810)) == iq16(-1920));

static_assert(math::signbit(iq16(0)) == 0);
static_assert(math::signbit(iq16(167)) == 0);
static_assert(math::signbit(iq16(-167)) == 1);
static_assert(math::signbit(iq31(0.5)) == 0);
static_assert(math::signbit(iq31(-0.5)) == 1);

static_assert(math::square(iq16(2)) = 4);
static_assert(math::square(uq29(2)) = 4);
static_assert(math::square(iq29(-2)) = 4);
static_assert(math::square(uq16(2)) = 4);
static_assert(math::square(uq16(100)) = 10000);


static_assert(math::sqrt(iq16(0)) == iq16(0));
static_assert(math::sqrt(iq16(4)) == iq16(2));
static_assert(math::sqrt(iq16(16)) == iq16(4));
static_assert(math::sqrt(iq16(64)) == iq16(8));
static_assert(math::sqrt(iq16(36)) == iq16(6));
static_assert(math::sqrt(iq16(16)) == iq16(4));

static_assert(math::sqrt(iiq16(0)) == iq16(0));
static_assert(math::sqrt(iiq16(4)) == iq16(2));
static_assert(math::sqrt(iiq16(16)) == iq16(4));
static_assert(math::sqrt(iiq16(64)) == iq16(8));
static_assert(math::sqrt(iiq16(36)) == iq16(6));
static_assert(math::sqrt(iiq16(16)) == iq16(4));

static_assert(math::sqrt(uq16(4)) == uq16(2));
static_assert(math::sqrt(uq16(16)) == uq16(4));

static constexpr int64_t err64(const uint64_t a, const uint64_t b){
    if(a > b) return a - b;
    else return b - a;
}
static_assert(err64(math::sqrt(uq32(0.25)).to_bits(), uq32(0.5).to_bits()) <= 4);
static_assert(err64(math::sqrt(uq32(1.0/16)).to_bits(), uq32(1.0/4).to_bits()) <= 4);
static_assert(err64(math::sqrt(uq32(1.0/64)).to_bits(), uq32(1.0/8).to_bits()) <= 4);
static_assert(err64(math::sqrt(uq32(1.0/256)).to_bits(), uq32(1.0/16).to_bits()) <= 4);

static_assert(math::inv_sqrt(iq10(16)) == iq10(0.25));
static_assert(math::inv_sqrt(iq16(16)) == iq16(0.25));
static_assert(math::inv_sqrt(uq16(16)) == uq16(0.25));

static_assert(math::mag(iq16(3), iq16(4)) == iq16(5));
static_assert(math::mag(iq26(3), iq26(4)) == iq26(5));

static_assert(math::inv_mag(iq16(3), iq16(4)).to_bits() == iq16(0.2).to_bits());
static_assert(math::inv_mag(iq16(12), iq16(5)).to_bits() == iq16(1.0/13).to_bits());

static_assert(math::mag(iq16(3), iq16(4)) == iq16(5));
static_assert(math::mag(iq26(3), iq26(4)) == iq26(5));

static_assert(math::inv_mag(iq16(3), iq16(4)).to_bits() == iq16(0.2).to_bits());
static_assert(math::inv_mag(iq16(12), iq16(5)).to_bits() == iq16(1.0/13).to_bits());

static_assert(math::mag(iq16(0), iq16(0), iq16(0)) == iq16(0));
static_assert(math::mag(iq16(1), iq16(2), iq16(2)) == iq16(3));
static_assert(math::mag(iq16(3), iq16(4), iq16(0)) == iq16(5));
static_assert(math::mag(iq16(1), iq16(1), iq16(1), iq16(1)) == iq16(2));

static_assert(math::inv_mag(iq16(1), iq16(2), iq16(2)).to_bits() == iq16(1.0/3).to_bits());

static_assert(math::sinpu(iq16(0)) == 0);
static_assert(math::sinpu(iq16(0.25)) == std::numeric_limits<iq31>::max());
static_assert(math::sinpu(iq16(0.5)) == iq16(0));
static_assert(math::sinpu(iq16(1.0)) == iq16(0));

static_assert(math::sinpu(iq10(0)) == iq10(0));
static_assert(math::sinpu(iq10(0.25)) == std::numeric_limits<iq31>::max());
static_assert(math::sinpu(iq10(0.5)) == iq10(0));
static_assert(math::sinpu(iq10(1.0)) == iq10(0));

static_assert(math::sinpu(iq30(0)) == iq30(0));
static_assert(math::sinpu(iq30(0.25)) == std::numeric_limits<iq31>::max());
static_assert(math::sinpu(iq30(0.5)) == iq30(0));
static_assert(math::sinpu(iq30(1.0)) == iq30(0));

[[maybe_unused]] static void test_exp(){
    constexpr float f0 = float(math::exp(iq16(0)));
    constexpr float f0_5 = float(math::exp(iq16(0.5)));
    constexpr float f1 = float(math::exp(iq16(1)));
    constexpr float f3 = float(math::exp(iq16(3)));
    static_assert(f0 == 1.0);
    static_assert((f0_5 - 1.6487212707) < 1e-4);
    static_assert((f1 - 2.7182818284590451) < 1e-4);
    static_assert((f3 - 20.0855369232) < 1e-4);

}

static_assert(std::abs((double)math::atan2pu(ymd::literals::iq16(1.0), ymd::literals::iq16(1.0)) - 0.125) < 1E-7);
static_assert(std::abs((double)math::atan2pu(-ymd::literals::iq16(1.0), -ymd::literals::iq16(1.0)) - 0.625) < 1E-7);
static_assert(std::abs((double)math::atan2pu(ymd::literals::iq16(1.0), -ymd::literals::iq16(1.0)) - 0.375) < 1E-7);
static_assert(std::abs((double)math::atan2pu(-ymd::literals::iq16(1.0), ymd::literals::iq16(1.0)) - 0.875) < 1E-7);

static_assert(std::abs((double)math::atanpu(ymd::literals::iq16(1.0)) - 0.125) < 1E-7);
static_assert(std::abs((double)math::atanpu(-ymd::literals::iq16(1.0)) - 0.875) < 1E-7);

static_assert(std::abs((double)math::asin(ymd::literals::iq16(1.0)) - (M_PI / 2)) < 1E-7);
static_assert(std::abs((double)math::asin(-ymd::literals::iq16(1.0)) - (-M_PI / 2)) < 1E-7);

static_assert(std::abs((double)math::asin(ymd::literals::iq16(0.5)) - (M_PI / 6)) < 1E-7);

}