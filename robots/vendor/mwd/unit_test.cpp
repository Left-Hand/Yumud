#include "mwd_msgs.hpp"
#include "mwd_utils.hpp"

using namespace ymd::robots::mwd;


namespace{

[[maybe_unused]] void test_crc(){
    {
        constexpr uint8_t val = WrapAddAccumulator()
            .push_byte(0x08)
            .finalize()
        ;
        static_assert(val == 0x08);
    }

    {
        constexpr uint8_t arr[] = {0x12, 0x34, 0x56};
        constexpr uint8_t val = WrapAddAccumulator()
            .push_bytes(std::span(arr))
            .finalize()
        ;
        static_assert(val == 0x12 + 0x34 + 0x56);
    }
}
}