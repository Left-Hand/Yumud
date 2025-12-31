#include "bytes_caster.hpp"

namespace {

using namespace ymd;

[[maybe_unused]] void endian_test(){
    {
        constexpr uint8_t bytes[2] = {0x12, 0x34};
        constexpr auto i = le_bytes_to_int<uint16_t>(std::span(bytes));
        static_assert(i == 0x12 + (0x34 << 8));
    }

    {
        constexpr uint8_t bytes[2] = {0x12, 0x34};
        constexpr auto i = be_bytes_to_int<uint16_t>(std::span(bytes));
        static_assert(i == 0x34 + (0x12 << 8));
    }

    {
        constexpr uint8_t bytes[4] = {0x12, 0x34, 0x56, 0x78};
        constexpr auto i = le_bytes_to_int<uint32_t>(std::span(bytes));
        static_assert(i == 0x78563412);
    }

    {
        constexpr uint8_t bytes[4] = {0x12, 0x34, 0x56, 0x78};
        constexpr auto i = be_bytes_to_int<uint32_t>(std::span(bytes));
        static_assert(i == 0x12345678);
    }
}
}