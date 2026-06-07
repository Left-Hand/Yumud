#include "../myactuator_primitive.hpp"
#include "../myactuator_msgs.hpp"
#include "../myactuator_frame_factory.hpp"

using namespace ymd;
using namespace ymd::robots::myactuator;

namespace {

static constexpr auto c = float(uq22(LapAngleCode_u16(35999).to_angle().to_turns()) * 360);
static constexpr auto turns = float(PositionCode_i32(0x7fffffff).to_angle().to_turns());


}