#pragma once


#include "../uavcan_primitive.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"

// https://cn.tmotor.com/uploadfile/2024/1101/20241101052544211.pdf

namespace ymd::uavcan::msgs{

[[nodiscard]] static constexpr uint8_t pushsci_verify(const uint8_t last, std::span<const uint8_t> bytes){
    uint32_t sum = last;

    #pragma GCC unroll 8
    for(size_t i = 0; i < bytes.size(); i++){
        sum += bytes[i];
    }
    return static_cast<uint8_t>(sum);
}

enum class PushSciFrameId:uint8_t{
    ReqSetZero = 0x09,
    ReqSetFocParaments = 0x1e,
    ReqGetFocParaments = 0x1a,
    RespControlFrame = 0x06
};

enum class EscId:uint8_t{

};

struct PushSciFrameHeader final{
    const uint8_t header_token1 = 0xec;
    const uint8_t header_token2 = 0x96;
    PushSciFrameId frame_id;
    uint8_t frame_ttl;
    EscId esc_id;
    uint8_t len;
};
}