#include "crsf_msgs.hpp"
#include "crsf_utils.hpp"
#include <charconv>

using namespace ymd::crsf;

namespace{


// [[maybe_unused]]static void test_u8_seq(){
//     constexpr std::array<uint8_t, 10> bytes = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
//     constexpr auto seq = U8Sequence::from_bytes(std::span(bytes));
//     // std::to_chars
//     static_assert(seq.size() == 10);
//     static_assert(seq.as_chars()[0] == 0x01);
// }

}