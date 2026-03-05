#pragma once

#include "core/utils/Result.hpp"
#include "algo/encrypt/crc_common.hpp"

//参考:
// https://cloud.tencent.com/developer/article/1940927
namespace ymd::ymodem{

enum class [[nodiscard]] Command:uint8_t{
    Soh = 0x01,
    Stx = 0x02,
    Eot = 0x04,
    Ack = 0x06,
    Nack = 0x15,
    Can = 0x18,
    Crc = 0x43
};

[[nodiscard]] static constexpr uint16_t calc_crc16(const std::span<const uint8_t> bytes){
    return encrypt::crc::crc16_ccitt(bytes);
}
}