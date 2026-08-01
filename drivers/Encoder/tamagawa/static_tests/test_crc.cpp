#include "../tamagawa_utils.hpp"
#include "../tamagawa_primitive.hpp"
#include "../tamagawa_msgs.hpp"
#include "../tamagawa_serialize.hpp"
#include "../tamagawa_api_facade.hpp"

#include "core/utils/Result.hpp"
#include <atomic>
#include <memory>

using namespace ymd;
using namespace ymd::drivers::tamagawa;

[[maybe_unused]] static void test_crc(){
    {
        static constexpr uint8_t CRC_VALUE = ChecksumBuilder::from_default()
            .push_byte(0x8a)
            .finalize();
        static_assert(CRC_VALUE == 0x8a);
    }

    // Test CRC for WriteEEprom request: [0x32, 0x01, 0x69] -> CRC should be 0x5a
    {
        static constexpr uint8_t CRC_VALUE = ChecksumBuilder::from_default()
            .push_byte(0x32)
            .push_byte(0x01)
            .push_byte(0x69)
            .finalize();
        static_assert(CRC_VALUE == 0x5a);
    }

    // Test CRC for ReadEEprom request: [0xea, 0x01] -> CRC should be 0xeb
    {
        static constexpr uint8_t CRC_VALUE = ChecksumBuilder::from_default()
            .push_byte(0xea)
            .push_byte(0x01)
            .finalize();
        static_assert(CRC_VALUE == 0xeb);
    }
}
