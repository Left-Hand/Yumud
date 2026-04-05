#pragma once

#include "mwd_primitive.hpp"


namespace ymd::robots::mwd::params{


enum class [[nodiscard]] ParamId : uint8_t {
    AngleLoopPid     = 0x0A,  // 10
    SpeedLoopPid     = 0x0B,  // 11
    CurrentLoopPid   = 0x0C,  // 12
    MaxTorqueCurrent = 0x1E, // 30
    MaxSpeed         = 0x20, // 32
    AngleLimit       = 0x22, // 34
    CurrentRamp      = 0x24, // 36
    SpeedRamp        = 0x26, // 38
};

// [0x0a]角度环 PID
struct [[nodiscard]] AngleLoopPid final{
    static constexpr ParamId PARAM_ID = ParamId::AngleLoopPid;

    uint16_t kp;
    uint16_t ki;
    uint16_t kd;

    template <typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize(Receiver &receiver) const {
        const std::array<uint8_t, 6> buf = {
            static_cast<uint8_t>(kp),
            static_cast<uint8_t>(kp >> 8),
            static_cast<uint8_t>(ki),
            static_cast<uint8_t>(ki >> 8),
            static_cast<uint8_t>(kd),
            static_cast<uint8_t>(kd >> 8),
        };
        if (auto res = receiver.push_bytes(std::span{buf}); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
};

// [0x0b]速度环 PID
struct [[nodiscard]] SpeedLoopPid final{
    static constexpr ParamId PARAM_ID = ParamId::SpeedLoopPid;

    uint16_t kp;
    uint16_t ki;
    uint16_t kd;

    template <typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize(Receiver &receiver) const {
        const std::array<uint8_t, 6> buf = {
            static_cast<uint8_t>(kp),
            static_cast<uint8_t>(kp >> 8),
            static_cast<uint8_t>(ki),
            static_cast<uint8_t>(ki >> 8),
            static_cast<uint8_t>(kd),
            static_cast<uint8_t>(kd >> 8),
        };
        if (auto res = receiver.push_bytes(std::span{buf}); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
};

// [0x0c]电流环 PID
struct [[nodiscard]] CurrentLoopPid final{
    static constexpr ParamId PARAM_ID = ParamId::CurrentLoopPid;

    uint16_t kp;
    uint16_t ki;
    uint16_t kd;

    template <typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize(Receiver &receiver) const {
        const std::array<uint8_t, 6> buf = {
            static_cast<uint8_t>(kp),
            static_cast<uint8_t>(kp >> 8),
            static_cast<uint8_t>(ki),
            static_cast<uint8_t>(ki >> 8),
            static_cast<uint8_t>(kd),
            static_cast<uint8_t>(kd >> 8),
        };
        if (auto res = receiver.push_bytes(std::span{buf}); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
};

// [0x01e]最大力矩电流
struct [[nodiscard]] MaxTorqueCurrent final{
    static constexpr ParamId PARAM_ID = ParamId::MaxTorqueCurrent;

    int16_t value;

    template <typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize(Receiver &receiver) const {
        // 协议：放在 byte3、byte4，前两字节填 0x00
        const std::array<uint8_t, 6> buf = {
            0x00,
            0x00,
            static_cast<uint8_t>(value),
            static_cast<uint8_t>(value >> 8),
            0x00,
            0x00,
        };
        if (auto res = receiver.push_bytes(std::span{buf}); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
};

// [0x20]最大速度
struct [[nodiscard]] MaxSpeed final{
    static constexpr ParamId PARAM_ID = ParamId::MaxSpeed;

    int32_t value;

    template <typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize(Receiver &receiver) const {
        // 协议：放在 byte3~6，前两字节填 0x00
        const std::array<uint8_t, 6> buf = {
            0x00,
            0x00,
            static_cast<uint8_t>(value),
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value >> 16),
            static_cast<uint8_t>(value >> 24),
        };
        if (auto res = receiver.push_bytes(std::span{buf}); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
};

// [0x22]角度限制
struct [[nodiscard]] AngleLimit final{
    static constexpr ParamId PARAM_ID = ParamId::AngleLimit;

    int32_t value;

    template <typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize(Receiver &receiver) const {
        const std::array<uint8_t, 6> buf = {
            0x00,
            0x00,
            static_cast<uint8_t>(value),
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value >> 16),
            static_cast<uint8_t>(value >> 24),
        };
        if (auto res = receiver.push_bytes(std::span{buf}); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
};

// [0x24]电流斜率
struct [[nodiscard]] CurrentRamp final{
    static constexpr ParamId PARAM_ID = ParamId::CurrentRamp;

    int32_t value;

    template <typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize(Receiver &receiver) const {
        const std::array<uint8_t, 6> buf = {
            0x00,
            0x00,
            static_cast<uint8_t>(value),
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value >> 16),
            static_cast<uint8_t>(value >> 24),
        };
        if (auto res = receiver.push_bytes(std::span{buf}); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
};

// [0x26]速度斜率
struct [[nodiscard]] SpeedRamp final{
    static constexpr ParamId PARAM_ID = ParamId::SpeedRamp;

    int32_t value;

    template <typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize(Receiver &receiver) const {
        const std::array<uint8_t, 6> buf = {
            0x00,
            0x00,
            static_cast<uint8_t>(value),
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value >> 16),
            static_cast<uint8_t>(value >> 24),
        };
        if (auto res = receiver.push_bytes(std::span{buf}); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
};
}