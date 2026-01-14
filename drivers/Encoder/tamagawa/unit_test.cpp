#include "tamagawa_utils.hpp"
#include "tamagawa_primitive.hpp"
#include <atomic>
#include <memory>

using namespace ymd::drivers::tamagawa;
using namespace utils;
namespace {
consteval size_t test_calc_crc() {
    // 使用 CRC-8 多项式: x^8 + 1 (0x81), LSB-first
    
    // 1. Test empty data
    std::array<uint8_t, 0> empty{};
    if (calc_crc8(empty) != 0x00) return 1;
    
    // 2. Test with single byte [0x00]
    std::array<uint8_t, 1> zero_byte{0x00};
    // 计算: 0x00 XOR, 然后8次移位都不会异或多项式
    // 最终结果: 0x00
    if (calc_crc8(zero_byte) != 0x00) return 2;
    
    return 0;
}

consteval size_t test_calc_xor() {
    // Test with empty data - should return 0
    std::array<uint8_t, 0> empty_data{};
    if(calc_xor(empty_data) != 0) return 1;
    
    // Test with single byte [0xAA]
    std::array<uint8_t, 1> single_byte{0xAA};
    if(calc_xor(single_byte) != 0xAA) return 2;
    
    // Test with multiple bytes that cancel out [0x55, 0xAA]
    std::array<uint8_t, 2> cancel_bytes{0x55, 0xAA}; // 0x55 ^ 0xAA = 0xFF
    if(calc_xor(cancel_bytes) != 0xFF) return 3;
    
    // Test with multiple bytes [0x01, 0x02, 0x03]
    std::array<uint8_t, 3> multi_bytes{0x01, 0x02, 0x03};
    if(calc_xor(multi_bytes) != 0x00) return 4; // 0x01 ^ 0x02 ^ 0x03 = 0x00
    
    return 0;
}

// Compile-time assertions
static_assert(test_calc_crc() == 0, "CRC calculation test failed");
static_assert(test_calc_xor() == 0, "XOR calculation test failed");

}

// std::atomic_ref<uint8_t> tamagawa_utils::crc_table[256] = {0};