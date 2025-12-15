#include "iqmath.hpp"

using namespace ymd;
using namespace ymd::iqmath;
using namespace ymd::math;
using namespace ymd::literals;

static_assert((iq16(4) >> 1) == iq16(2));
static_assert((iq16::from_rcp(4)) == iq16(0.25));
static_assert((uq16::from_rcp(4u)) == uq16(0.25));
static_assert((uuq32::from_rcp(32768u) * 32768u).to_bits() == uuq32(1).to_bits());
static_assert((iq16(1) + iq16(2)) == iq16(3));
static_assert((iq16(2) + iq16(1)) == iq16(3));
static_assert((iq16(2) * iq16(1)) == iq16(2));

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

static_assert(inv_sqrt(iq16(16)) == iq16(0.25));
static_assert(inv_sqrt(iq16(25)) == iq16(0.2));
static_assert(mag(iq16(3), iq16(4)) == iq16(5));
static_assert(inv_mag(iq16(3), iq16(4)).to_bits() == iq16(0.2).to_bits());

static_assert(math::sinpu(iq16(0)) == iq16(0));
static_assert(math::sinpu(iq16(0.25)) == std::numeric_limits<iq31>::max());
static_assert(math::sinpu(iq16(0.5)) == iq16(0));
// static_assert(math::sinpu(iq16(0.75)).to_bits() == std::numeric_limits<iq31>::min().to_bits());
static_assert(math::sinpu(iq16(1.0)) == iq16(0));

static_assert(frac(iq16(0.25)) == iq16(0.25));
static_assert(frac(iq16(1.7)) == iq16(0.7));


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

static_assert(signbit(iq16(0)) == 0);
static_assert(signbit(iq16(167)) == 0);
static_assert(signbit(iq16(-167)) == 1);

static_assert(square(iq16(2)) = 4);
