#include "m1502e_primitive.hpp"

using namespace ymd::robots::bmkj::m1502e;

// static_assert(PositionCode::from_angle())


static_assert(sizeof(CurrentCode) == 2);
static_assert(CurrentCode::from_amps(34).unwrap_err() == std::weak_ordering::greater);
static_assert(CurrentCode::from_amps(-34).unwrap_err() == std::weak_ordering::less);
static_assert(std::abs(static_cast<float>(CurrentCode::from_bits(32767).to_amps()) - 33.0f) < 1E-2);
static_assert(std::abs(static_cast<float>(CurrentCode::from_bits(-32767).to_amps()) - -33.0f) < 1E-2);


namespace{

}