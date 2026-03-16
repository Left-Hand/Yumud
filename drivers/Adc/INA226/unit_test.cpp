#include "ina226.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


using Self = INA226;

template<typename T>
requires (std::is_integral_v<T>)
static constexpr T diff(T a, T b){
    return a > b ? a - b : b - a;
}



// static constexpr auto f = (float)sv_code_to_volts(volts_to_sv_code(-0.08_iq16));
// static constexpr auto f = (float)bv_code_to_volts(0x7fff);
// static constexpr auto f = (float)bv_code_to_volts(0x8300);


static_assert(Self::volts_to_sv_code(-0.08_iq16) == 0x8300);
static_assert(Self::mv_to_sv_code(-80) == 0x8300);
static_assert(Self::uv_to_sv_code(-80'000) == 0x8300);

static_assert(std::abs((float)Self::sv_code_to_volts(0x8300) - (-0.08)) < 1E-4);
static_assert(Self::sv_code_to_mv(0x8300) == -80);
static_assert(Self::sv_code_to_uv(0x8300) == -80'000);


static_assert(std::abs((float)Self::bv_code_to_volts(0x7fff) - (40.96)) < 2E-4);
static_assert(diff(Self::bv_code_to_mv(0x7fff), uint32_t(40.96 * 1000)) < 2);
static_assert(diff(Self::bv_code_to_uv(0x7fff), uint32_t(40.96 * 1000'000)) < 10);


    //    _1 = 0,
    //     _4 = 1,
    //     _16 = 2,
    //     _64 = 3,
    //     _128 = 4,
    //     _256 = 5,
    //     _512 = 6,
    //     _1024 = 7



static_assert(Self::times_to_avtimes(1) == Self::AverageTimes::_1);
static_assert(Self::times_to_avtimes(4) == Self::AverageTimes::_4);
static_assert(Self::times_to_avtimes(16) == Self::AverageTimes::_16);
static_assert(Self::times_to_avtimes(64) == Self::AverageTimes::_64);
static_assert(Self::times_to_avtimes(128) == Self::AverageTimes::_128);
static_assert(Self::times_to_avtimes(256) == Self::AverageTimes::_256);
static_assert(Self::times_to_avtimes(512) == Self::AverageTimes::_512);
static_assert(Self::times_to_avtimes(1024) == Self::AverageTimes::_1024);