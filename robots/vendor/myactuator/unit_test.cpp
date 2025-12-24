#include "myactuator_primitive.hpp"

using namespace ymd::robots::myactuator;

namespace {

#if 0
[[maybe_unused]] static void static_test(){
    static constexpr auto bytes = std::to_array<uint8_t>({0x12, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07});
    static constexpr resp_msgs::SetTorque obj = resp_msgs::SetTorque::from_bytes(std::span(bytes));
    static_assert(obj.motor_temperature.bits == 0x12);
    // static constexpr auto it = le_bytes_into_bits(std::span(bytes));
    // static constexpr PositionCode_i32 temp = (it);
}
#endif

static constexpr auto c = float(uq22(LapAngleCode_u16(35999).to_angle().to_turns()) * 360);
static constexpr auto turns = float(PositionCode_i32(0x7fffffff).to_angle().to_turns());

}