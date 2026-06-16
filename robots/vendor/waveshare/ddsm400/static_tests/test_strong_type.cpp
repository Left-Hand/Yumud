#include "../ddsm400_transport.hpp"
#include "../ddsm400_msgs.hpp"
#include "../ddsm400_factory.hpp"

using namespace ymd;
using namespace ymd::robots::waveshare::ddsm400;

namespace {

static_assert(sizeof(SpeedCode) == 2);

static_assert(SpeedCode::try_from_rpm(10).unwrap().bits == 100);
static_assert(SpeedCode::try_from_rpm(-10).unwrap().bits == -100);

static_assert(math::abs((float)SpeedCode::try_from_rpm(-10).unwrap().to_rpm() - -10.0) < 1E-4);
static_assert(SpeedCode::try_from_rpm(10).unwrap().to_rpm() == 10);


static_assert(CurrentCode::try_from_amps(-2).unwrap().bits == -0x4000);
static_assert(CurrentCode::try_from_amps(-4).unwrap().bits == -0x8000);
static_assert(CurrentCode::try_from_amps(4).unwrap().bits == 0x7fff);

static_assert(CurrentCode::try_from_amps(-2).unwrap().to_amps() == -2);
static_assert(CurrentCode::try_from_amps(-4).unwrap().to_amps() == -4);
static_assert(CurrentCode::try_from_amps(2).unwrap().to_amps() == 2);
static_assert(math::abs((float)CurrentCode::try_from_amps(4).unwrap().to_amps() - 4.0f) < 1.4e-4);

static_assert(CurrentCode::try_from_amps(2).unwrap().bits == 0x4000);
static_assert(LapAngleCode::from_angle(Angular<uq32>::from_turns(0.25_uq32)).bits == 0x4000);

}