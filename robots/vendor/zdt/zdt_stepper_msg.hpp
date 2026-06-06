#pragma once

#include "zdt_stepper_primitive.hpp"
#include "zdt_stepper_utils.hpp"

namespace ymd::robots::zdtmotor{
    
namespace req_msgs{
// 地址 + 0xF3 + 0xAB + 使能状态 + 多机同步标志 + 校验字节
struct [[nodiscard]] Actvation final{
    static constexpr FuncCode FUNC_CODE = FuncCode::Activation;
    static constexpr size_t PAYLOAD_LENGTH = 3;

    bool en;
    bool is_sync;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> buf) const noexcept {
        buf[0] = 0xab;
        buf[1] = en;
        buf[2] = is_sync;
    }
};

struct [[nodiscard]] SetPosition final{
    static constexpr FuncCode FUNC_CODE = FuncCode::SetPosition;
    static constexpr size_t PAYLOAD_LENGTH = 9;
    bool is_ccw;
    Rpm rpm;
    AcclerationLevel acc_level;
    PulseCnt pulse_cnt;
    bool is_absolute; //9
    bool is_sync;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> buf) const noexcept {
        BytesFiller filler(buf);
        filler.push_le_u8(is_ccw);
        filler.push_le_u16(rpm.bits);
        filler.push_le_u8(static_cast<uint8_t>(acc_level.bits));
        filler.push_le_u32(pulse_cnt.bits);
        filler.push_le_u8(is_absolute);
        filler.push_le_u8(is_sync);
    }
};

struct [[nodiscard]] SetSpeed final{
    static constexpr FuncCode FUNC_CODE = FuncCode::SetSpeed;
    static constexpr size_t PAYLOAD_LENGTH = 7;
    bool is_ccw;//2
    Rpm rpm;//3-4
    AcclerationLevel acc_level;
    bool is_absolute; //9
    bool is_sync; //10

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> buf) const noexcept {
        BytesFiller filler(buf);
        filler.push_le_u8(is_ccw);
        filler.push_le_u16(rpm.bits);
        filler.push_le_u8(static_cast<uint8_t>(acc_level.bits));
        filler.push_le_u8(acc_level.bits);
        filler.push_le_u8(is_absolute);
        filler.push_le_u8(is_sync);
    }
};

struct [[nodiscard]] SetSubDivides final{
    // 01 84 8A 01 07 6B
    //  0x84 + 0x8A + 是否存储标志 + 细分值 + 校验字节

    static constexpr FuncCode FUNC_CODE = FuncCode::SetSubDivide;
    static constexpr size_t PAYLOAD_LENGTH = 3;

    bool is_burned;
    uint8_t subdivides;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> buf) const noexcept {
        buf[0] = 0x8a;
        buf[1] = is_burned;
        buf[2] = subdivides;
    }
};

struct [[nodiscard]] Brake final{
    static constexpr FuncCode FUNC_CODE = FuncCode::Brake;
    static constexpr size_t PAYLOAD_LENGTH = 2;

    bool is_sync;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> buf) const noexcept {
        buf[0] = 0x98;
        buf[1] = is_sync;
    }
};

struct [[nodiscard]] TrigCali final{

    static constexpr FuncCode FUNC_CODE = FuncCode::TrigCali;
    static constexpr size_t PAYLOAD_LENGTH = 1;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> buf) const noexcept {
        buf[0] = 0x45;
    }
};

struct [[nodiscard]] SetCurrent final{
    static constexpr FuncCode FUNC_CODE = FuncCode::SetCurrent;//1
    static constexpr size_t PAYLOAD_LENGTH = 9;

    bool is_ccw;//2
    Rpm rpm;//3-4
    PulseCnt pulse_cnt;//5-8
    bool is_absolute; //9
    bool is_sync; //10

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> buf) const noexcept {
        BytesFiller filler(buf);
        filler.push_le_u8(is_ccw);
        filler.push_le_u16(rpm.bits);
        filler.push_le_u32(pulse_cnt.bits);
        filler.push_le_u8(is_absolute);
        filler.push_le_u8(is_sync);
    }
};

struct [[nodiscard]] TrigHomming final{
    static constexpr FuncCode FUNC_CODE = FuncCode::TrigHomming;
    static constexpr size_t PAYLOAD_LENGTH = 2;

    HommingMode homming_mode;
    bool is_sync;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> buf) const noexcept {
        buf[0] = static_cast<uint8_t>(homming_mode);
        buf[1] = is_sync;
    }
};

struct [[nodiscard]] QueryHommingParaments final{
    static constexpr FuncCode FUNC_CODE = FuncCode::QueryHommingParaments;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};


struct [[nodiscard]] QueryHommingStatus final{
    static constexpr FuncCode FUNC_CODE = FuncCode::QueryHommingParaments;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

}
}
