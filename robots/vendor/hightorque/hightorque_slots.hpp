#include "hightorque_utils.hpp"

namespace ymd::robots::hightorque{


namespace req_slots{
using namespace primitive;
using namespace utils;

static constexpr uint16_t NIL_U16 = 0x8000;

struct [[nodiscard]] SetMode{
    static constexpr size_t NUM_BYTES = 3;
    Mode mode;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const {
        SlotBuilder{SlotCommand::Write, RegAddr::Mode}.build(bytes, mode);
    }
};


struct [[nodiscard]] ControlQCurrent{
    static constexpr size_t NUM_BYTES = 4;
    CurrentCode q_current_code;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const {
        SlotBuilder{SlotCommand::Write, RegAddr{0x1c}}.build(bytes, q_current_code);
    }
};

struct [[nodiscard]] ControlPosition{
    static constexpr size_t NUM_BYTES = 8;
    PositionCode position_code;
    TorqueCode torque_code;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const {
        SlotBuilder{SlotCommand::Write, RegAddr{0x07}}.build(bytes, position_code, NIL_U16, torque_code);
    }
};

struct [[nodiscard]] ControlTorque{
    static constexpr size_t NUM_BYTES = 4;
    TorqueCode torque_code;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const {
        SlotBuilder{SlotCommand::Write,RegAddr{0x13}}.build(bytes, torque_code);
    }
};

// 电机位置-速度-最大力矩控制，int16型
struct [[nodiscard]] ControlPvt{
    static constexpr size_t NUM_BYTES = 8;
    PositionCode position_code;
    SpeedCode speed_code;
    TorqueCode torque_code;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const {
        SlotBuilder{SlotCommand::Write, RegAddr{0x35}}.build(bytes, position_code, speed_code, torque_code);
    }
};

// 梯形控制（电机固件 v4.6.0 开始支持）
struct [[nodiscard]] ControlTrapezoidal{
    static constexpr size_t NUM_BYTES = 8;
    PositionCode position_code;
    SpeedCode speed_code;
    AccelerationCode accel_code;
    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const {
        SlotBuilder{SlotCommand::Write, RegAddr{0x35}}.build(bytes, position_code, speed_code, accel_code);
    }
};

struct [[nodiscard]] RezeroPosition{

};


};
}