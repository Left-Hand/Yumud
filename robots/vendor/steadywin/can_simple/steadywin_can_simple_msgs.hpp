#pragma once

#include "steadywin_can_simple_primitive.hpp"

namespace ymd::robots::steadywin::can_simple{
namespace req_msgs{
struct [[nodiscard]] Heartbeat{
    using Self = Heartbeat;
    static constexpr CommandKind COMMAND =  Command::Heartbeat;
    AxisErrorFlags axis_error;
    AxisState axis_state;


    Flags flags;
    uint8_t __resv__;

    // 周期消息的生命值，每一个⼼跳消息加 1，范围
    // 0-255，如果此生命值不连续，表示⼼跳消息丢
    // 失，即通信不稳
    uint8_t life;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

static_assert(sizeof(Heartbeat) == 8);

// CMD ID: 0x002（主机→电机）⽆参数⽆数据。
// 此指令会导致电机紧急停机，并报 ESTOP_REQUESTED 异常
struct [[nodiscard]] Estop{
    static constexpr CommandKind COMMAND = Command::Estop;
};

struct [[nodiscard]] GetError{
    static constexpr CommandKind COMMAND = Command::GetMotorError;
};

// CMD ID: 0x003（电机→主机）
struct [[nodiscard]] GetErrorReq{
    using Self = GetErrorReq;
    static constexpr CommandKind COMMAND = Command::GetMotorError;
    ErrorType type;
};

// CMD ID: 0x003（电机←主机）
struct [[nodiscard]] GetErrorResp{
    using Self = GetErrorResp;
    static constexpr CommandKind COMMAND = Command::GetMotorError;
    union{
        uint64_t motor_exception;
        uint32_t encoder_exception;
        uint32_t controller_exception;
        uint32_t system_exception;
    };
    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }
};

static_assert(sizeof(GetErrorResp) == 8);   


// struct [[nodiscard]] EncoderCount{
//     static constexpr CommandKind COMMAND = Command::GetEncoderCount;
//     int32_t shadow_count;
//     int32_t cpr_count;
// };
struct [[nodiscard]] RxSdo{
    using Self = RxSdo;
    static constexpr CommandKind COMMAND = Command::RxSdo;
    bool is_read;
    uint32_t endpoint_id;
    uint8_t __resv__;
    uint32_t value;
};

struct [[nodiscard]] TxSdo{
    using Self = RxSdo;
    static constexpr CommandKind COMMAND = Command::TxSdo;
    bool is_read;
    uint32_t endpoint_id;
    uint8_t __resv__;
    uint32_t value;
};

// CMD ID: 0x006（主机→电机）
struct [[nodiscard]] SetAxisNodeId{
    using Self = SetAxisNodeId;
    static constexpr CommandKind COMMAND = Command::SetAxisNodeId;
    uint32_t node_id;
};


// CMD ID: 0x007（主机→电机）
struct [[nodiscard]] SetAxisState{
    using Self = SetAxisState;
    static constexpr CommandKind COMMAND = Command::SetAxisState;
    AxisState state;
};


struct [[nodiscard]] MitControl{
    
    using Self = MitControl;
    static constexpr CommandKind COMMAND = Command::MitControl;

    mit::MitPositionCode_u16 position;
    mit::MitSpeedCode_u12 speed;
    mit::MitKpCode_u12 kp;
    mit::MitKdCode_u12 kd;
    mit::MitTorqueCode_u12 torque;

    constexpr hal::BxCanPayload to_payload(){
        std::array<uint8_t, 8> bytes;
        bytes[0] = static_cast<uint8_t>(position.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(position.to_bits() & 0xff);
        bytes[2] = static_cast<uint8_t>(speed.to_bits() >> 4);
        bytes[3] = static_cast<uint8_t>(((speed.to_bits() & 0xf) << 4) | ((kp.to_bits() >> 8)));
        bytes[4] = static_cast<uint8_t>(kp.to_bits() & 0xff);
        bytes[5] = static_cast<uint8_t>(kd.to_bits() >> 4);
        bytes[6] = static_cast<uint8_t>(((kd.to_bits() & 0xf) << 4) | ((torque.to_bits() >> 8)));
        bytes[7] = static_cast<uint8_t>(torque.to_bits() & 0xf);
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(bytes));
    }

    constexpr Self from_bytes(std::span<const uint8_t, 8> bytes) const {
        const uint16_t position_bits = 
            (bytes[0] << 8) | bytes[1];
        const uint16_t speed_bits = 
            (bytes[2] << 4) | (bytes[3] >> 4);
        const uint16_t kp_bits = 
            ((bytes[3] & 0x0f) << 8) | (bytes[4]);
        const uint16_t kd_bits = 
            (bytes[5] << 4) | (bytes[6] >> 4);
        const uint16_t torque_bits = 
            ((bytes[6] & 0x0f) << 8) | (bytes[7]);
        return Self{
            .position = mit::MitPositionCode_u16::from_bits(position_bits),
            .speed = mit::MitSpeedCode_u12::from_bits(speed_bits),
            .kp = mit::MitKpCode_u12::from_bits(kp_bits),
            .kd = mit::MitKdCode_u12::from_bits(kd_bits),
            .torque = mit::MitTorqueCode_u12::from_bits(torque_bits)
        };
    };
};

//ID 0x009
struct [[nodiscard]] GetEncoderEstimates{
    using Self = GetEncoderEstimates;
    static constexpr CommandKind COMMAND = Command::GetEncoderEstimates;
    math::fp32 position;
    math::fp32 velocity;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

//ID 0x00A
struct [[nodiscard]] GetMotorCurrent{
    using Self = GetMotorCurrent;
    static constexpr CommandKind COMMAND = Command::GetMotorCurrent;
    int32_t shadow_count;
    int32_t count_n_cpr;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

//ID 0x00B
struct [[nodiscard]] SetCotrollerMode{
    using Self = SetCotrollerMode;
    static constexpr CommandKind COMMAND = Command::SetControllerMode;
    ControlMode control_mode;
    InputMode input_mode;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return Self {
            .control_mode = std::bit_cast<ControlMode>(payload[0]),
            .input_mode = std::bit_cast<InputMode>(payload[4])
        };
    }

    constexpr hal::BxCanPayload to_payload() const {
        //stupid padding
        std::array<uint8_t, 8> bytes = {
            std::bit_cast<uint8_t>(control_mode),
            0, 0, 0,
            std::bit_cast<uint8_t>(input_mode),
            0, 0, 0
        };
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(bytes));
    }
};

//ID 0x00C
struct [[nodiscard]] SetInputPosition{ 
    using Self = SetInputPosition;
    static constexpr Command COMMAND = CommandKind{0x00c};

    math::fp32 input_position;
    int16_t vel_ff;
    int16_t torque_ff;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetInputVelocity{ 
    using Self = SetInputVelocity;
    static constexpr Command COMMAND = CommandKind{0x00d};

    math::fp32 vel_ff;
    math::fp32 torque_ff;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetInputTorque{ 
    using Self = SetInputTorque;
    static constexpr Command COMMAND = CommandKind{0x00e};

    math::fp32 torque_ff;
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};


struct [[nodiscard]] SetLimits{
    using Self = SetLimits;
    static constexpr Command COMMAND = CommandKind{0x00f};

    math::fp32 velocity_limit;
    math::fp32 current_limit;
    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] StartAntiCogging{
    using Self = StartAntiCogging;
    static constexpr Command COMMAND = CommandKind{0x010};
};

struct [[nodiscard]] SetTrajVelLimit{
    using Self = SetTrajVelLimit;
    static constexpr Command COMMAND = CommandKind{0x011};

    math::fp32 traj_vel_limit;
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetTrajAccelLimit{ 
    using Self = SetTrajAccelLimit;
    static constexpr Command COMMAND = CommandKind{0x012};

    math::fp32 traj_accel_limit;
    math::fp32 traj_decel_limit;
    

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetTrajInertia{
    using Self = SetTrajInertia;
    static constexpr Command COMMAND = CommandKind{0x013};
    math::fp32 traj_inertia;//惯量
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] GetIq{ 
    using Self = GetIq;
    static constexpr Command COMMAND = CommandKind{0x014};
    math::fp32 id_setpoint;
    math::fp32 iq_measured;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] Reboot{ 
    using Self = Reboot;
    static constexpr Command COMMAND = CommandKind{0x016};
};

struct [[nodiscard]] GetBusVoltageCurrent{
    using Self = GetBusVoltageCurrent;
    static constexpr Command COMMAND = CommandKind{0x017};
    math::fp32 bus_voltage;
    math::fp32 bus_current;
    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};


struct [[nodiscard]] ClearErrors{ 
    using Self = ClearErrors;
    static constexpr Command COMMAND = CommandKind{0x018};
};

struct [[nodiscard]] SetLinearCount{
    using Self = SetLinearCount;
    static constexpr Command COMMAND = CommandKind{0x019};
    int32_t linear_count;
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};


struct [[nodiscard]] SetPosGain{
    using Self = SetPosGain;
    static constexpr Command COMMAND = CommandKind{0x01a};
    math::fp32 pos_gain;
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};


struct [[nodiscard]] SetVelGain{
    using Self = SetVelGain;
    static constexpr Command COMMAND = CommandKind{0x01b};
    math::fp32 vel_gain;
    math::fp32 vel_integrator_gain;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetTorques{
    using Self = SetTorques;
    static constexpr Command COMMAND = CommandKind{0x01c};
    math::fp32 torque_setpoint;
    math::fp32 torque;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] GetPowers{
    using Self = GetPowers;
    static constexpr Command COMMAND = CommandKind{0x01d};
    math::fp32 electrical_power;
    math::fp32 mechanical_power;

    static constexpr Self form_payload(const hal::BxCanPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::BxCanPayload to_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] DisableCan{
    using Self = DisableCan;
    static constexpr Command COMMAND = CommandKind{0x01e};
};

struct [[nodiscard]] SaveConfig{
    using Self = SaveConfig;
    static constexpr Command COMMAND = CommandKind{0x01f};
};



}

#if 0

struct [[nodiscard]] Signal:public Sumtype<
    // msgs::EncoderCount,
    // msgs::GetEncoderEstimates,
    msgs::Heartbeat
    // msgs::GetMotorCurrent,
    // msgs::VbusVoltage,
    // EncoderErrorFlags,
    // MotorErrorFlags
>{
    // using EncoderCount = msgs::EncoderCount;
    // using GetEncoderEstimates = msgs::GetEncoderEstimates;
    // using Heartbeat = msgs::Heartbeat;
    // using GetMotorCurrent = msgs::GetMotorCurrent;
    // using VbusVoltage = msgs::VbusVoltage;
    // using EncoderErrorFlags = msgs::EncoderErrorFlags;
    // using MotorErrorFlags = msgs::MotorErrorFlags;
};

struct [[nodiscard]] Event{
    AxisId axis_id;
    Signal signal;
};


#endif

}