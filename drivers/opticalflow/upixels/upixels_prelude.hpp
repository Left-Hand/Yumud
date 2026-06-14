#pragma once

#include <cstdint>
#include <span>

// 参考资料：
// https://github.com/Upixels-China/Upixels_Optical_flow/blob/main/demo/stm32_demo_code_LC302-3C/flow_decode.c


namespace ymd::drivers::upixels{

// n = sizeof(packet)
// 0xfe | 0x0a | [payload; n] | checksum(payload[0:n]) | 0x55


struct [[nodiscard]] Packet final{
    int16_t		flow_x_integral;
    int16_t		flow_y_integral;
    uint16_t   	integration_timespan;
    uint16_t   	ground_distance;
    uint8_t    	quality;
    uint8_t    	version;
};

static constexpr size_t NUM_BYTES = sizeof(Packet);

[[nodiscard]] static constexpr 
uint8_t calc_checksum(const std::span<const uint8_t> bytes){
    uint8_t sum = 0;
    const size_t len = bytes.size();
    for(size_t i = 0; i < len; i++){
        sum = sum ^ bytes[i];
    }
    return sum;
}

enum class FsmState{
    AwaitHeader1,
    AwaitHeader2,
    
}

}
