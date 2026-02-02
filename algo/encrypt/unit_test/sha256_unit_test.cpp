#include "../sha256.hpp"


namespace {

#if 0
static constexpr void test_sha256(){ 
    constexpr auto input = std::array<uint8_t, 5>{0x61, 0x62, 0x63, 0x64, 0x65};
    constexpr auto digest = calculate_sha256(std::span(input));
    // static_assert(digest[0] == 0);
}
#endif

}