#include "stl06n.hpp"

using namespace ymd::drivers::stl06n;

namespace {
[[maybe_unused]] static void test_crc(){
    constexpr uint8_t bytes[] = {
        0x54, 0x2C, 0x68, 0x08, 0xAB, 0x7E, 0xE0, 0x00,
        0xE4, 0xDC, 0x00, 0xE2, 0xD9, 0x00, 0xE5, 0xD5,
        0x00, 0xE3, 0xD3, 0x00, 0xE4, 0xD0, 0x00, 0xE9,
        0xCD, 0x00, 0xE4, 0xCA, 0x00, 0xE2, 0xC7, 0x00,
        0xE9, 0xC5, 0x00, 0xE5, 0xC2, 0x00, 0xE5, 0xC0,
        0x00, 0xE5, 0xBE, 0x82, 0x3A, 0x1A
    };

    constexpr auto actual_crc = [&]() -> uint8_t{
        Crc8Calculator calc = Crc8Calculator();
        return calc.push_bytes(std::span(bytes)).get();
    }();
    static_assert(actual_crc == 0x50);
}



static_assert(__builtin_offsetof(LidarSectorPacket, LidarSectorPacket::spin_speed) == 0);
static_assert(__builtin_offsetof(LidarSectorPacket, LidarSectorPacket::start_angle) == 2);
static_assert(__builtin_offsetof(LidarSectorPacket, LidarSectorPacket::points) == 4);
static_assert(__builtin_offsetof(LidarSectorPacket, LidarSectorPacket::stop_angle) == 40);
static_assert(__builtin_offsetof(LidarSectorPacket, LidarSectorPacket::timestamp) == 42);
static_assert(__builtin_offsetof(LidarSectorPacket, LidarSectorPacket::crc8) == 44);


}
