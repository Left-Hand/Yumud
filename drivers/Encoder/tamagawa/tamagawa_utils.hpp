#pragma once

#include "tamagawa_primitive.hpp"

namespace ymd::drivers::tamagawa::utils{

// 多项式：G(X)=X^8+1 LSB first  Poly: 0000 0001
// LSB first  : 1000 0000 =0X80
[[nodiscard]] static constexpr uint8_t calc_crc(std::span<const uint8_t> bytes) {
    uint8_t crc = 0;
    
    for (size_t i = 0; i < bytes.size(); i++) {
        uint8_t data = bytes[i];
        
        for (int j = 0; j < 8; j++) {
            uint8_t val = ((data >> 7) & 0x01) ^ ((crc >> 7) & 0x01);
            crc = (crc << 1) & 0xFF;
            data = (data << 1) & 0xFF;
            crc |= val;
        }
    }
    
    return crc;
}
[[nodiscard]] static constexpr uint8_t calc_xor(std::span<const uint8_t> bytes){
    uint8_t sum = 0;

    #pragma GCC unroll 4
    for(size_t i = 0; i < bytes.size(); i++){
        sum ^= bytes[i];
    }

    return sum;
}
}