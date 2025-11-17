#pragma once

#include <cstdint>
#include <span>

namespace ymd::uavcan{

namespace crc_func{
constexpr uint16_t crc_addbyte(uint16_t crc_val, uint8_t byte){
    crc_val ^= (uint16_t) ((uint16_t) (byte) << 8);
    for (uint8_t j= 0; j < 8; j++){
        if (crc_val & 0x8000U){
        crc_val = (uint16_t) ((uint16_t) (crc_val << 1) ^ 0x1021U);
        }else{
        crc_val = (uint16_t) (crc_val << 1);
        }
    }
    return crc_val;
}
constexpr uint16_t crc_add_signature(uint16_t crc_val, uint64_t data_type_signature){
    for (uint16_t shift_val = 0; shift_val <64; shift_val =(uint16_t)(shift_val +8U)){
        crc_val =crc_addbyte(crc_val, (uint8_t) (data_type_signature >> shift_val));
    }
    return crc_val;
}


}

struct CrcBuilder{
    uint16_t val;

    constexpr void add_byte(const uint8_t byte){
        val = crc_func::crc_addbyte(val, byte);
    }

    constexpr void add_bytes(const std::span<const uint8_t> bytes){
        for (const auto byte : bytes){
            add_byte(byte);
        }
    }

    constexpr void add_signature(const uint64_t signature){
        val = crc_func::crc_add_signature(val, signature);
    }

    constexpr uint16_t build() const{
        return val;
    }
};
}