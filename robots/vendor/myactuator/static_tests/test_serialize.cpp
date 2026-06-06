#include "../myactuator_primitive.hpp"
#include "../myactuator_msgs.hpp"
#include "../myactuator_frame_factory.hpp"

using namespace ymd;
using namespace ymd::robots::myactuator;

namespace {

#if 1
[[maybe_unused]] static void test_deserialize(){
    {
        static constexpr auto bytes = std::to_array<uint8_t>({0x12, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07});
        static constexpr resp_msgs::SetTorque obj = resp_msgs::SetTorque::try_from_bytes(std::span(bytes)).unwrap();
        static_assert(obj.motor_temperature.bits == 0x12);

    }  
}
#endif


}