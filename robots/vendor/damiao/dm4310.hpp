#pragma once

#include "../utils.hpp"
#include "core/math/fixed/fixed.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/math/float/fp32.hpp"
#include "robots/vendor/mit/mit_primitive.hpp"
#include "prelude.hpp"

namespace ymd::robots::damiao::dm4310{


struct [[nodiscard]] PosVelParam final{
    math::fp32 q;
    math::fp32 dq;

    constexpr hal::BxCanPayload to_can_payload() const {
        auto & self = *this;

        const auto pos_bytes = std::bit_cast<std::array<uint8_t, 4>>(self.q);

        const auto vel_bytes = std::bit_cast<std::array<uint8_t, 4>>(self.dq);

        return hal::BxCanPayload::from_u8x8({
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

struct [[nodiscard]] PosVelParam final{
    math::fp32 dq;
    constexpr hal::BxCanPayload to_can_payload() const {
        auto & self = *this;

        const auto vel_bytes = std::bit_cast<std::array<uint8_t, 4>>(self.dq);

        return hal::BxCanPayload::from_list({
            vel_bytes[0],
            vel_bytes[1],
            vel_bytes[2],
            vel_bytes[3],
        });
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

    constexpr std::array<uint8_t, 2> to_bytes() const{ 
        return {uint8_t(bits & 0xFF), uint8_t(bits >> 8)};
    }

    constexpr float to_rps() const{
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

    constexpr std::array<uint8_t, 2> to_bytes() const{ 
        return {uint8_t(bits & 0xFF), uint8_t(bits >> 8)};
    }


    constexpr float to_perunit() const{
        return float(bits) / 10000.0f;
    }
};

struct PosForceParam{
    // P_des: position command in rad (float).

    math::fp32 q;
    QdCode qd_code;
    TorqueCurrentLimitCode torque_current_limit_code;

    constexpr hal::BxCanPayload to_can_payload() const {
        auto & self = *this;

        const auto pos_bytes = std::bit_cast<std::array<uint8_t, 4>>(self.q);

        const auto vel_bytes = self.qd_code.to_bytes();
        const auto torque_bytes = self.torque_current_limit_code.to_bytes();

        return hal::BxCanPayload::from_u8x8({
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

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(position.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(position.to_bits() & 0xff);
        bytes[2] = static_cast<uint8_t>(speed.to_bits() >> 4);
        bytes[3] = static_cast<uint8_t>(((speed.to_bits() & 0xf) << 4) | ((kp.to_bits() >> 8)));
        bytes[4] = static_cast<uint8_t>(kp.to_bits() & 0xff);
        bytes[5] = static_cast<uint8_t>(kd.to_bits() >> 4);
        bytes[6] = static_cast<uint8_t>(((kd.to_bits() & 0xf) << 4) | ((torque.to_bits() >> 8)));
        bytes[7] = static_cast<uint8_t>(torque.to_bits() & 0xf);
    };

    constexpr hal::BxCanPayload to_can_payload() const{
        std::array<uint8_t, 8> bytes;
        fill_bytes(bytes);
        return hal::BxCanPayload::from_u8x8(bytes);
    }
};


struct FrameFactory{

    const hal::CanStdId motor_can_id;

    // 7.2.1 使能、失能、保存零点、清除错误
    // 电机需要发送使能命令后，电机LED由红变绿之后才可以进行控制，无论使用哪种模
    // 式，使能电机的命令都是一样的
    constexpr hal::BxCanFrame enable() const {
        return hal::BxCanFrame::from_parts(motor_can_id, pack_command_data(0xFC));
    }

    constexpr hal::BxCanFrame disable() const {
        return hal::BxCanFrame::from_parts(motor_can_id, pack_command_data(0xFD));
    }

    // 保存零点
    constexpr hal::BxCanFrame set_zero() const {
        return hal::BxCanFrame::from_parts(motor_can_id, pack_command_data(0xFE));
    }

    //清除错误
    constexpr hal::BxCanFrame clear_error() const {
        return hal::BxCanFrame::from_parts(motor_can_id, pack_command_data(0xFC));
    }

    constexpr hal::BxCanFrame mit_control(const MitParams& mit_param) const {
        return hal::BxCanFrame::from_parts(motor_can_id, mit_param.to_can_payload());
    }


    constexpr hal::BxCanFrame posvel_control(const PosVelParam& posvel_param) const {
        // pos vel mode needs extra 0x100
        return hal::BxCanFrame::from_parts(POS_VEL_MODE + motor_can_id, posvel_param.to_can_payload());
    }

    constexpr hal::BxCanFrame posforce_control(const PosForceParam& posforce_param) {
        // pos force mode needs extra 0x300
        return hal::BxCanFrame::from_parts(POS_FORCE_MODE + motor_can_id, posforce_param.to_can_payload());
    }

    template<typename T>
    requires (sizeof(T) == 4)
    constexpr hal::BxCanFrame write_param(const uint8_t reg_addr, const T param){
        const auto && param_bytes = std::bit_cast<std::array<uint8_t, 4>(param);
        return pack_write_param(motor_can_id, reg_addr, param_bytes); 
    }

    constexpr hal::BxCanFrame query_param(uint8_t reg_addr) const {

        return hal::BxCanFrame::from_parts(NMT_CAN_FRAME_ID, pack_query_param_data(motor_can_id, reg_addr));
    }

private:
    struct [[nodiscard]] IdBase final{
        uint16_t count;

        constexpr hal::CanStdId  operator +(const hal::CanStdId stdid) const {
            return hal::CanStdId::from_u11(count + stdid.to_u11());
        }
    };

    static constexpr IdBase POS_VEL_MODE = {0x100};
    static constexpr IdBase VEL_ONLY_MODE = {0x200};
    static constexpr IdBase POS_FORCE_MODE = {0x300};
    static constexpr auto NMT_CAN_FRAME_ID = hal::CanStdId::from_u11(0x7FF);

    static constexpr hal::BxCanPayload pack_command_data(const uint8_t b) {
        const auto arr = std::array<uint8_t, 8>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, b};
        return hal::BxCanPayload::from_u8x8(arr);
    }

    static constexpr hal::BxCanPayload pack_query_param_data(const hal::CanStdId send_can_id, uint8_t reg_addr){
        const uint16_t id_u11 = send_can_id.to_u11();
        const auto arr = std::array<uint8_t, 8>{
            static_cast<uint8_t>(id_u11 & 0xFF),
            static_cast<uint8_t>((id_u11 >> 8) & 0xFF),
            0x33,
            reg_addr,
            0x00, 0x00, 0x00, 0x00
        };
        return hal::BxCanPayload::from_u8x8(arr);
    }


    static constexpr hal::BxCanFrame pack_write_param(
        const hal::CanStdId motor_can_id, 
        const uint8_t reg_addr, 
        std::array<uint8_t, 4> bytes
    ) {
        const uint16_t id_u11 = motor_can_id.to_u11();
        const auto arr = std::array<uint8_t, 8>{
            static_cast<uint8_t>(id_u11 & 0xFF),
            static_cast<uint8_t>((id_u11 >> 8) & 0xFF),
            0x55,
            reg_addr,
            bytes[0], bytes[1], bytes[2], bytes[3]
        };
        return hal::BxCanFrame::from_parts(NMT_CAN_FRAME_ID, hal::BxCanPayload::from_u8x8(arr));
    }

    static constexpr hal::BxCanFrame pack_save_param(
        const hal::CanStdId motor_can_id
    ) {
        const uint16_t id_u11 = motor_can_id.to_u11();
        const auto arr = std::array<uint8_t, 8>{
            static_cast<uint8_t>(id_u11 & 0xFF),
            static_cast<uint8_t>((id_u11 >> 8) & 0xFF),
            0xaa,
            0x01,
            0x00, 0x00, 0x00, 0x00
        };
        return hal::BxCanFrame::from_parts(NMT_CAN_FRAME_ID, hal::BxCanPayload::from_u8x8(arr));
    }
};



}