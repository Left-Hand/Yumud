#include "damiao_primitive.hpp"
#include "core/math/float/fp32.hpp"
#include "robots/vendor/mit/mit_primitive.hpp"

namespace ymd::robots::damiao{

struct [[nodiscard]] PosVelParam final{
    math::fp32 q;
    math::fp32 dq;

    constexpr hal::ClassicCanPayload to_can_payload() const noexcept {
        auto & self = *this;

        const auto pos_bytes = std::bit_cast<std::array<uint8_t, 4>>(self.q);

        const auto vel_bytes = std::bit_cast<std::array<uint8_t, 4>>(self.dq);

        return hal::ClassicCanPayload::from_u8x8({
            pos_bytes[0],
            pos_bytes[1],
            pos_bytes[2],
            pos_bytes[3],
            vel_bytes[0],
            vel_bytes[1],
            vel_bytes[2],
            vel_bytes[3]
        });
    }
};  

struct [[nodiscard]] VelParam final{
    math::fp32 dq;


    constexpr hal::ClassicCanPayload to_can_payload() const noexcept {
        auto & self = *this;

        const auto vel_bytes = std::bit_cast<std::array<uint8_t, 4>>(self.dq);

        return hal::ClassicCanPayload::from_bytes(vel_bytes);
    }
};  


// V_des: speed limit in rad/s, scaled by 100 into uint16 (little-endian), range 0-10000
// (values above 10000 clamp to 10000), corresponding to 0-100 rad/s.

struct [[nodiscard]] QdCode final{
    using Self = QdCode;

    uint16_t bits;

    static constexpr Self from_rps(const float rps){ 
        return Self{std::bit_cast<uint16_t>(uint16_t(rps * 100.0f))};
    }

    constexpr std::array<uint8_t, 2> to_bytes() const noexcept { 
        return {uint8_t(bits & 0xFF), uint8_t(bits >> 8)};
    }

    constexpr float to_rps() const noexcept {
        return float(bits) / 100.0f;
    }
};

// I_des: torque current limit per-unit value, scaled by 10000 into uint16 (little-endian),
// range 0-10000 (values above 10000 clamp to 10000), corresponding to 0-1.0.
// Per-unit current value = actual current / max current (Imax printed on power-up).
struct [[nodiscard]] TorqueCurrentLimitCode final{
    using Self = TorqueCurrentLimitCode;
    uint16_t bits;

    static constexpr Self from_perunit(const float x){
        return Self{static_cast<uint16_t>(x * 10000.0f)};
    }

    constexpr std::array<uint8_t, 2> to_bytes() const noexcept { 
        return {uint8_t(bits & 0xFF), uint8_t(bits >> 8)};
    }


    constexpr float to_perunit() const noexcept {
        return float(bits) / 10000.0f;
    }
};

struct PosForceParam{
    // P_des: position command in rad (float).

    math::fp32 q;
    QdCode qd_code;
    TorqueCurrentLimitCode torque_current_limit_code;

    constexpr hal::ClassicCanPayload to_can_payload() const noexcept {
        auto & self = *this;

        const auto pos_bytes = std::bit_cast<std::array<uint8_t, 4>>(self.q);

        const auto vel_bytes = self.qd_code.to_bytes();
        const auto torque_bytes = self.torque_current_limit_code.to_bytes();

        return hal::ClassicCanPayload::from_u8x8({
            pos_bytes[0],
            pos_bytes[1],
            pos_bytes[2],
            pos_bytes[3],
            vel_bytes[0], vel_bytes[1],
            torque_bytes[0], torque_bytes[1]
        });
    }
};


struct [[nodiscard]] MitParams final{
    mit::MitPositionCode_u16 position;
    mit::MitSpeedCode_u12 speed;
    mit::MitKpCode_u12 kp;
    mit::MitKdCode_u12 kd;
    mit::MitTorqueCode_u12 torque;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const noexcept {
        bytes[0] = static_cast<uint8_t>(position.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(position.to_bits() & 0xff);
        bytes[2] = static_cast<uint8_t>(speed.to_bits() >> 4);
        bytes[3] = static_cast<uint8_t>(((speed.to_bits() & 0xf) << 4) | ((kp.to_bits() >> 8)));
        bytes[4] = static_cast<uint8_t>(kp.to_bits() & 0xff);
        bytes[5] = static_cast<uint8_t>(kd.to_bits() >> 4);
        bytes[6] = static_cast<uint8_t>(((kd.to_bits() & 0xf) << 4) | ((torque.to_bits() >> 8)));
        bytes[7] = static_cast<uint8_t>(torque.to_bits() & 0xf);
    };

    constexpr hal::ClassicCanPayload to_can_payload() const noexcept {
        std::array<uint8_t, 8> bytes;
        fill_bytes(bytes);
        return hal::ClassicCanPayload::from_u8x8(bytes);
    }
};


template<typename T>
struct [[nodiscard]] FeedbackPacketInterpreter final{
    alignas(4) T bytes;

    // 获取状态
    [[nodiscard]] constexpr NodeId motor_id() const {
        return NodeId((bytes[0] & 0xF));
    }

    // 获取状态
    [[nodiscard]] constexpr Status status() const {
        return Status::from_bits((bytes[0] & 0xF0) >> 4);
    }

    // 获取角度值 (u16)
    [[nodiscard]] constexpr uint16_t angle_u16() const {
        return (bytes[1] << 8) | bytes[2];
    }

    // 获取速度值 (u12)
    [[nodiscard]] constexpr uint16_t speed_u12() const {
        return (bytes[3] << 4) | (bytes[4] >> 4);
    }

    // 获取扭矩值 (u12)
    [[nodiscard]] constexpr uint16_t torque_u12() const {
        return ((bytes[4] & 0x0F) << 8) | bytes[5];
    }

    // 获取MOS温度
    [[nodiscard]] constexpr int8_t mos_temperature() const {
        return static_cast<int8_t>(bytes[6]);
    }

    // 获取电机温度
    [[nodiscard]] constexpr int8_t motor_temperature() const {
        return static_cast<int8_t>(bytes[7]);
    }

};

using FeedbackPacket = FeedbackPacketInterpreter<std::array<uint8_t, 8>>;

}