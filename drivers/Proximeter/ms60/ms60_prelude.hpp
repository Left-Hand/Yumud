
#pragma once

#include <cstdint>
#include <span>

namespace ymd::drivers{


struct MS60_Prelude{
    static constexpr size_t MS60_FRAME_HEAD1        = 0x5A;    // 帧头第1字节
    static constexpr size_t NUM_MS60_TARGET_MAX         = 8;       // 最大目标数
    static constexpr size_t MS60_TARGET_DATA_LEN    = 4;       // 每个目标数据长度 (距离1B + 角度1B + 速度1B + ID1B)
    static constexpr size_t MS60_FRAME_TYPE         = 0x07;    // 帧类型
    static constexpr size_t MS60_PAYLOAD_HEADER_LEN = 5;       // Payload中目标数据前的字节数 (TYPE 1B + obj_num 2B + reserved 2B)
    // 最大帧长度 = HEAD(1B) + LEN(1B) + PAYLOAD + CHECK(1B)
    static constexpr size_t MS60_MAX_FRAME_LEN      = 
        (1 + 1 + MS60_PAYLOAD_HEADER_LEN + NUM_MS60_TARGET_MAX * MS60_TARGET_DATA_LEN + 1);

    // 单个目标数据结构
    struct TargetInfo{
        uint8_t distance;
        int8_t angle;
        int8_t speed;
        uint8_t id;
    };

    // 检测数据结构
    struct Packet{
        std::array<TargetInfo, NUM_MS60_TARGET_MAX> targets;
        uint16_t obj_num;
    };
}



}