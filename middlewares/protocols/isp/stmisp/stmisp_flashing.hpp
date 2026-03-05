#pragma once

#include "core/utils/Result.hpp"


//参考资料：
//https://github.com/nicekwell/stm32ISP/blob/master/stm32isp.c


namespace ymd::stmisp{

static constexpr Ack parse_ack(const uint8_t b){
    constexpr uint8_t ACK_MAGIC = 0x79;
    if(b == ACK_MAGIC) return ACK;
    return NACK;
};

static constexpr uint8_t calc_checksum(std::span<const uint8_t> bytes)      //计算p开始len个字节的checksum，也就是计算异或
{
    uint8_t cs = 0;
    for (size_t i = 0; i < bytes.size(); i++){
        cs ^= bytes[i];
    }
    return cs;
}



}