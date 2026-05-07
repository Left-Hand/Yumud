#include "myactuator_primitive.hpp"
#include "myactuator_msgs.hpp"
#include "myactuator_frame_factory.hpp"

using namespace ymd;
using namespace ymd::robots::myactuator;

namespace {

static constexpr auto c = float(uq22(LapAngleCode_u16(35999).to_angle().to_turns()) * 360);
static constexpr auto turns = float(PositionCode_i32(0x7fffffff).to_angle().to_turns());

#if 1
[[maybe_unused]] static void test_deser(){
    {
        static constexpr auto bytes = std::to_array<uint8_t>({0x12, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07});
        static constexpr resp_msgs::SetTorque obj = resp_msgs::SetTorque::try_from_bytes(std::span(bytes)).unwrap();
        static_assert(obj.motor_temperature.bits == 0x12);

    }
}
#endif


}