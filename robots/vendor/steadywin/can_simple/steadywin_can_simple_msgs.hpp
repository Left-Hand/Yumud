#pragma once

#include "steadywin_can_simple_primitive.hpp"

namespace ymd::robots::steadywin::can_simple{
namespace req_msgs{



// CMD ID: 0x002（主机→电机）⽆参数⽆数据。
// 此指令会导致电机紧急停机，并报 ESTOP_REQUESTED 异常
struct [[nodiscard]] Estop final{
    static constexpr CommandKind COMMAND = Command::Estop;
};

// CMD ID: 0x003
struct [[nodiscard]] GetError final{
    using Self = GetError;
    static constexpr CommandKind COMMAND = Command::GetMotorError;
    ErrorSource source;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(source);
    }
};

//ID 0x004
struct [[nodiscard]] RxSdo final{
    using Self = RxSdo;
    static constexpr CommandKind COMMAND = Command::RxSdo;
    bool is_read;
    uint32_t endpoint_id;
    uint8_t __resv__;
    uint32_t value;
};

//ID 0x005
struct [[nodiscard]] TxSdo final{
    using Self = RxSdo;
    static constexpr CommandKind COMMAND = Command::TxSdo;
    bool is_read;
    uint32_t endpoint_id;
    uint8_t __resv__;
    uint32_t value;
};

// CMD ID: 0x006（主机→电机）
struct [[nodiscard]] SetAxisNodeId final{
    using Self = SetAxisNodeId;
    static constexpr CommandKind COMMAND = Command::SetAxisNodeId;
    uint32_t axis_node_id;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_le_u32(axis_node_id);
    }
};

// CMD ID: 0x007（主机→电机）
struct [[nodiscard]] SetAxisState final{
    using Self = SetAxisState;
    static constexpr CommandKind COMMAND = Command::SetAxisState;
    AxisState axis_state;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(axis_state);
    }
};

//ID 0x008
struct [[nodiscard]] MitControl final{
    
    using Self = MitControl;
    static constexpr CommandKind COMMAND = Command::MitControl;

    mit::MitPositionCode_u16 position;
    mit::MitSpeedCode_u12 speed;
    mit::MitKpCode_u12 kp;
    mit::MitKdCode_u12 kd;
    mit::MitTorqueCode_u12 torque;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes){
        bytes[0] = static_cast<uint8_t>(position.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(position.to_bits() & 0xff);
        bytes[2] = static_cast<uint8_t>(speed.to_bits() >> 4);
        bytes[3] = static_cast<uint8_t>(((speed.to_bits() & 0xf) << 4) | ((kp.to_bits() >> 8)));
        bytes[4] = static_cast<uint8_t>(kp.to_bits() & 0xff);
        bytes[5] = static_cast<uint8_t>(kd.to_bits() >> 4);
        bytes[6] = static_cast<uint8_t>(((kd.to_bits() & 0xf) << 4) | ((torque.to_bits() >> 8)));
        bytes[7] = static_cast<uint8_t>(torque.to_bits() & 0xf);
    }

    constexpr Self try_from_bytes(std::span<const uint8_t, 8> bytes) const {
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


#if 0
//ID 0x00A
struct [[nodiscard]] GetMotorCurrent final{
    using Self = GetMotorCurrent;
    static constexpr CommandKind COMMAND = Command::GetMotorCurrent;
    int32_t shadow_count;
    int32_t count_n_cpr;

    static constexpr Self from_can_payload(const hal::BxCanPayload & can_payload){
        return std::bit_cast<Self>(can_payload.u8x8());
    }

    constexpr hal::BxCanPayload to_can_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};
#endif

//ID 0x00B
struct [[nodiscard]] SetCotrollerMode final{
    using Self = SetCotrollerMode;
    static constexpr CommandKind COMMAND = Command::SetControllerMode;
    LoopMode loop_mode;
    InputMode input_mode;

    static constexpr Self from_can_payload(const hal::BxCanPayload & can_payload){
        return Self {
            .loop_mode = std::bit_cast<LoopMode>(can_payload[0]),
            .input_mode = std::bit_cast<InputMode>(can_payload[4])
        };
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        //stupid padding
        std::fill_n(bytes.begin(), 8, 0);
        bytes[0] = std::bit_cast<uint8_t>(loop_mode);
        bytes[4] = std::bit_cast<uint8_t>(input_mode);
    }
};

//ID 0x00C
struct [[nodiscard]] SetInputPosition final{
    using Self = SetInputPosition;
    static constexpr Command COMMAND = CommandKind{0x00c};

    math::fp32 input_position;
    int16_t vel_ff;
    int16_t torque_ff;

    // static constexpr Self from_can_payload(const hal::BxCanPayload & can_payload){
    //     return std::bit_cast<Self>(can_payload.u8x8());
    // }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(input_position);
        filler.push_le_i16(vel_ff);
        filler.push_le_i16(torque_ff);
    }
};

//ID 0x00d
struct [[nodiscard]] SetInputVelocity final{
    using Self = SetInputVelocity;
    static constexpr Command COMMAND = CommandKind{0x00d};

    math::fp32 vel_ff;
    math::fp32 torque_ff;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(vel_ff);
        filler.push_fp32(torque_ff);
    }
};

//ID 0x00e
struct [[nodiscard]] SetInputTorque final{
    using Self = SetInputTorque;
    static constexpr Command COMMAND = CommandKind{0x00e};

    math::fp32 torque_ff;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(torque_ff);
    }
};

//ID 0x00f
struct [[nodiscard]] SetLimits final{
    using Self = SetLimits;
    static constexpr Command COMMAND = CommandKind{0x00f};

    math::fp32 velocity_limit;
    math::fp32 current_limit;
    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(velocity_limit);
        filler.push_fp32(current_limit);
    }
};

//ID 0x010
struct [[nodiscard]] StartAntiCogging final{
    using Self = StartAntiCogging;
    static constexpr Command COMMAND = CommandKind{0x010};
};

//ID 0x011
struct [[nodiscard]] SetTrajVelLimit final{
    using Self = SetTrajVelLimit;
    static constexpr Command COMMAND = CommandKind{0x011};

    math::fp32 traj_vel_limit;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(traj_vel_limit);
    }
};

//ID 0x012
struct [[nodiscard]] SetTrajAccelLimit final{
    using Self = SetTrajAccelLimit;
    static constexpr Command COMMAND = CommandKind{0x012};

    math::fp32 traj_accel_limit;
    math::fp32 traj_decel_limit;
    

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(traj_accel_limit);
        filler.push_fp32(traj_decel_limit);
    }
};

//ID 0x013
struct [[nodiscard]] SetTrajInertia final{
    using Self = SetTrajInertia;
    static constexpr Command COMMAND = CommandKind{0x013};
    math::fp32 traj_inertia;//惯量

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(traj_inertia);
    }
};

//ID 0x014
struct [[nodiscard]] GetIq final{
    using Self = GetIq;
    static constexpr Command COMMAND = CommandKind{0x014};
    math::fp32 id_setpoint;
    math::fp32 iq_measured;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(id_setpoint);
        filler.push_fp32(iq_measured);
    }
};

//ID 0x016
struct [[nodiscard]] Reboot final{
    using Self = Reboot;
    static constexpr Command COMMAND = CommandKind{0x016};
};

//ID 0x017
struct [[nodiscard]] GetBusVoltageCurrent final{
    using Self = GetBusVoltageCurrent;
    static constexpr Command COMMAND = CommandKind{0x017};
    math::fp32 bus_voltage;
    math::fp32 bus_current;
    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(bus_voltage);
        filler.push_fp32(bus_current);
    }
};

//ID 0x018
struct [[nodiscard]] ClearErrors final{
    using Self = ClearErrors;
    static constexpr Command COMMAND = CommandKind{0x018};
};

//ID 0x019
struct [[nodiscard]] SetLinearCount final{
    using Self = SetLinearCount;
    static constexpr Command COMMAND = CommandKind{0x019};
    int32_t linear_count;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_le_i32(linear_count);
    }
};

//ID 0x01a
struct [[nodiscard]] SetPosGain final{
    using Self = SetPosGain;
    static constexpr Command COMMAND = CommandKind{0x01a};
    math::fp32 pos_gain;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(pos_gain);
    }
};

//ID 0x01b
struct [[nodiscard]] SetVelGain final{
    using Self = SetVelGain;
    static constexpr Command COMMAND = CommandKind{0x01b};
    math::fp32 vel_gain;
    math::fp32 vel_integrator_gain;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(vel_gain);
        filler.push_fp32(vel_integrator_gain);
    }
};

//ID 0x01c
struct [[nodiscard]] SetTorques final{
    using Self = SetTorques;
    static constexpr Command COMMAND = CommandKind{0x01c};
    math::fp32 torque_setpoint;
    math::fp32 torque_measured;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(torque_setpoint);
        filler.push_fp32(torque_measured);
    }
};

//ID 0x01d
struct [[nodiscard]] GetPowers final{
    using Self = GetPowers;
    static constexpr Command COMMAND = CommandKind{0x01d};
    math::fp32 electrical_power;
    math::fp32 mechanical_power;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_fp32(electrical_power);
        filler.push_fp32(mechanical_power);
    }
};

//ID 0x01e
struct [[nodiscard]] DisableCan final{
    using Self = DisableCan;
    static constexpr Command COMMAND = CommandKind{0x01e};
};

//ID 0x01f
struct [[nodiscard]] SaveConfig final{
    using Self = SaveConfig;
    static constexpr Command COMMAND = CommandKind{0x01f};
};

}

namespace resp_msgs{

// CMD ID: 0x001（电机←主机）
struct [[nodiscard]] HeartbeatV513 final{
    using Self = HeartbeatV513;
    static constexpr CommandKind COMMAND =  Command::Heartbeat;

    AxisFaultFlags axis_fault_flags;
    AxisState axis_state;


    MotorFlags motor_flags;

    // 周期消息的生命值，每一个⼼跳消息加 1，范围
    // 0-255，如果此生命值不连续，表示⼼跳消息丢
    // 失，即通信不稳
    uint8_t life;

    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> & bytes){
        const auto self = Self{
            .axis_fault_flags = std::bit_cast<AxisFaultFlags>(le_bytes_to_int<uint32_t>(bytes.subspan<0, 4>())),
            .axis_state = ({
                const auto res = try_into_axis_state(bytes[4]); 
                if(res.is_err()) return Err(res.unwrap_err());
                res.unwrap();
            }),
            .motor_flags = std::bit_cast<MotorFlags>(bytes[5]),
            .life = bytes[7]
        };

        return Ok(self);
    }
};


// CMD ID: 0x003
struct [[nodiscard]] GetError final{
    using Self = GetError;
    static constexpr CommandKind COMMAND = Command::GetMotorError;
    union{
        uint64_t motor_exception; 
        uint32_t encoder_exception;
        uint32_t controller_exception;
        uint32_t system_exception;
    };
    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self;
        self.motor_exception = le_bytes_to_int<uint64_t>(bytes);
        return Ok(self);
    }
};

static_assert(sizeof(GetError) == 8);   

//ID 0x009
struct [[nodiscard]] GetEncoderEstimates final{
    using Self = GetEncoderEstimates;
    static constexpr CommandKind COMMAND = Command::GetEncoderEstimates;
    math::fp32 position;
    math::fp32 velocity;

    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self{
            .position = std::bit_cast<math::fp32>(le_bytes_to_int<uint32_t>(bytes.subspan<0, 4>())),
            .velocity = std::bit_cast<math::fp32>(le_bytes_to_int<uint32_t>(bytes.subspan<4, 4>()))
        };
        return Ok(self);
    }
};

//ID 0x00a
struct [[nodiscard]] GetEncoderCount final{
    using Self = GetEncoderCount;
    static constexpr CommandKind COMMAND = Command::GetEncoderCount;
    int32_t shadow_count;
    int32_t cpr_count;

    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self{
            .shadow_count = le_bytes_to_int<int32_t>(bytes.subspan<0, 4>()),
            .cpr_count = le_bytes_to_int<int32_t>(bytes.subspan<4, 4>())
        };
        return Ok(self);
    }
};


}



template<typename T>
static constexpr hal::BxCanFrame serialize_msg_to_can_frame(
    const AxisId & axis_id, 
    const T & msg
){
    constexpr auto COMMAND = T::COMMAND;
    const FrameId frame_id = FrameId{
        .axis_id = axis_id,
        .command = COMMAND,
    };

    std::array<uint8_t, 8> bytes;
    if constexpr(std::is_same_v<
        decltype(msg.fill_bytes(bytes)), void>
    ){
        msg.fill_bytes(bytes);
    }
    return hal::BxCanFrame::from_parts(
        frame_id.to_stdid(),
        hal::BxCanPayload::from_u8x8(bytes)
    );
}



}