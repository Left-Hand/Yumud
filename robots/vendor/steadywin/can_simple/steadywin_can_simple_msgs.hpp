#pragma once

#include "steadywin_can_simple_primitive.hpp"
#include "steadywin_can_simple_utils.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "core/utils/bytes/bytes_caster.hpp"
#include <utility>

namespace ymd::robots::steadywin::can_simple{
namespace req_msgs{
using namespace steadywin::primitive;
// struct SpeedForwardCode{
//     int16_t bits;

//     static constexpr SpeedForwardCode from_tps(const )
//     {
//         return SpeedForwardCode{speed};
//     }
// }

// CMD ID: 0x002（主机→电机）⽆参数⽆数据。
// 此指令会导致电机紧急停机，并报 ESTOP_REQUESTED 异常
struct [[nodiscard]] Estop final{
    static constexpr CommandKind COMMAND = Command::Estop;
};

// CMD ID: 0x003
struct [[nodiscard]] GetError final{
    using Self = GetError;
    static constexpr CommandKind COMMAND = Command::GetError;
    ErrorSource source;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(source);
    }
};

struct [[nodiscard]] GetEncoderCount final{
    using Self = GetEncoderCount;
    static constexpr CommandKind COMMAND = Command::GetEncoderCount;

};

//ID 0x004
struct [[nodiscard]] RxSdo final{
    using Self = RxSdo;
    static constexpr CommandKind COMMAND = Command::RxSdo;
    bool is_read;
    uint32_t endpoint_id;
    uint32_t value_bits;
};

//ID 0x005
struct [[nodiscard]] TxSdo final{
    using Self = RxSdo;
    static constexpr CommandKind COMMAND = Command::TxSdo;
    bool is_read;
    uint32_t endpoint_id;
    uint32_t value_bits;
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

    constexpr Result<Self, DeMsgError> try_from_bytes(std::span<const uint8_t, 8> bytes) const {
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
        return Ok(Self{
            .position = mit::MitPositionCode_u16::from_bits(position_bits),
            .speed = mit::MitSpeedCode_u12::from_bits(speed_bits),
            .kp = mit::MitKpCode_u12::from_bits(kp_bits),
            .kd = mit::MitKdCode_u12::from_bits(kd_bits),
            .torque = mit::MitTorqueCode_u12::from_bits(torque_bits)
        });
    };
};


#if 0
//ID 0x00A
struct [[nodiscard]] GetMotorCurrent final{
    using Self = GetMotorCurrent;
    static constexpr CommandKind COMMAND = Command::GetMotorCurrent;
    int32_t multilap_angle;
    int32_t count_n_cpr;

    constexpr hal::BxCanPayload to_can_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};
#endif

//ID 0x00B
struct [[nodiscard]] SetControllerMode final{
    using Self = SetControllerMode;
    static constexpr CommandKind COMMAND = Command::SetControllerMode;
    LoopMode loop_mode;
    InputMode input_mode;

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
struct [[nodiscard]] SetMoveIncremental final{
    using Self = SetMoveIncremental;
    static constexpr Command COMMAND = CommandKind{0x019};
    math::fp32 displacement;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        filler.push_le_i32(displacement.to_bits());
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
// struct [[nodiscard]] SetTorques final{
//     using Self = SetTorques;
//     static constexpr Command COMMAND = CommandKind{0x01c};
//     math::fp32 torque_setpoint;
//     math::fp32 torque_measured;

//     constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
//         BytesFiller filler(bytes);
//         filler.push_fp32(torque_setpoint);
//         filler.push_fp32(torque_measured);
//     }
// };

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
using namespace steadywin::primitive;
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

    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        const auto self = Self{
            .axis_fault_flags = std::bit_cast<AxisFaultFlags>(
                le_bytes_to_int<uint32_t>(bytes.subspan<0, 4>())
            ),
            .axis_state = ({
                const auto res = try_into_axis_state(bytes[4]); 
                if(res.is_err()) return Err(res.unwrap_err());
                res.unwrap();
            }),
            .motor_flags = MotorFlags::from_bits(bytes[5]),
            .life = bytes[7]
        };

        return Ok(self);
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) {
        return os 
            << os.field("axis_fault_flags")(self.axis_fault_flags) << os.splitter()
            << os.field("axis_state")(self.axis_state) << os.splitter()
            << os.field("motor_flags")(self.motor_flags) << os.splitter()
            << os.field("life")(self.life);
    }
};


// CMD ID: 0x003
struct [[nodiscard]] GetError final{
    using Self = GetError;
    static constexpr CommandKind COMMAND = Command::GetError;
    union{
        uint64_t motor_exception; 
        uint32_t encoder_exception;
        uint32_t controller_exception;
        uint32_t system_exception;
    };
    __no_inline static constexpr Result<Self, DeMsgError> 
    try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self;
        self.motor_exception = le_bytes_to_int<uint64_t>(bytes);
        return Ok(self);
    }
    friend OutputStream & operator<<(OutputStream & os, const Self & self) {
        return os;
    }
};

static_assert(sizeof(GetError) == 8);   

//id 0x008
struct MitControl{
    using Self = MitControl;
    static constexpr CommandKind COMMAND = Command::MitControl;
    uint8_t node_id;

    // 位置：总共 16 位，BYTE1 为高 8 位，BYTE2 为低 8 位
    // 输出轴的多圈位置，单位为弧度（RAD），范围-mit_max_pos～mit_max_pos
    // 实际位置为 double 型，需要转换为 16 位 int 型，转换过程为：
    // pos_int = (pos_double + mit_max_pos)*65535 / 25
    // mit_max_pos默认值同上
    mit::MitPositionCode_u16 position_code;

    // 速度：总共 12 位，BYTE3 为其高 8 位，BYTE4[7-4]（高 4 位）为其低 4 位。表示输出轴的角速度，单位为 RAD/s，范围
    // -mit_max_vel~ mit_max_vel
    mit::MitSpeedCode_u12 speed_code;

    // 力矩：总共 12 位，BYTE4[3-0]（低 4 位）为其高 4 位，BYTE5 为其低 8 位。单位是 N.m。
    // 范围-mit_max_torque～mit_max_torque
    mit::MitTorqueCode_u12 torque_code;

    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 5> bytes){
        const uint16_t position_code_bits = le_bytes_to_int<uint16_t>(bytes.subspan<1, 2>());
        const int16_t speed_code_bits = 
            (static_cast<int16_t>(bytes[3]) << 4)
            | (static_cast<int16_t>(bytes[4]) >> 4);

        const int16_t torque_code_bits = 
            (static_cast<int16_t>(bytes[5]))
            | (static_cast<int16_t>(bytes[4] & 0x0f) << 4);
        Self self{
            .node_id = bytes[0],
            .position_code = mit::MitPositionCode_u16::from_bits(position_code_bits),
            .speed_code = mit::MitSpeedCode_u12::from_bits(speed_code_bits),
            .torque_code = mit::MitTorqueCode_u12::from_bits(torque_code_bits)
        };
        return Ok(self);
    }
};

//ID 0x009
struct [[nodiscard]] GetEncoderEstimates final{
    using Self = GetEncoderEstimates;
    static constexpr CommandKind COMMAND = Command::GetEncoderEstimates;
    math::fp32 position;
    math::fp32 velocity;

    __no_inline  static constexpr Result<Self, DeMsgError> 
    try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self{
            // .position = std::bit_cast<math::fp32>(le_bytes_to_int<uint32_t>(bytes.subspan<0, 4>())),
            // .velocity = std::bit_cast<math::fp32>(le_bytes_to_int<uint32_t>(bytes.subspan<4, 4>()))
            .position = std::bit_cast<math::fp32>(le_bytes_to_int<uint32_t>(bytes.subspan<0, 4>())),
            .velocity = std::bit_cast<math::fp32>(le_bytes_to_int<uint32_t>(bytes.subspan<4, 4>()))
        };
        return Ok(self);
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) {
        return os << os.field("position")(self.position) << os.splitter()
            << os.field("velocity")(self.velocity);
    }
};

//ID 0x00a
struct [[nodiscard]] GetEncoderCount final{
    using Self = GetEncoderCount;
    static constexpr CommandKind COMMAND = Command::GetEncoderCount;
    Angular<iq14> multilap_angle;
    Angular<uq32> lap_angle;

    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        const iq14 multilap_turns = iq14::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<0, 4>()));
        const uq32 lap_turns = uq32::from_bits(le_bytes_to_int<uint32_t>(bytes.subspan<4, 4>()) << (32 - 14));
        Self self{
            .multilap_angle = Angular<iq14>::from_turns(multilap_turns),
            .lap_angle = Angular<uq32>::from_turns(lap_turns),
        };
        return Ok(self);
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) {
        return os << os.field("multilap_angle")(self.multilap_angle) << os.splitter()
            << os.field("lap_angle")(self.lap_angle);
    }
};

//0x14
struct [[nodiscard]] GetIq final{
    using Self = GetIq;
    static constexpr CommandKind COMMAND = Command::GetIq;
    math::fp32 iq_setpoint;
    math::fp32 iq_measured;
    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self{
            .iq_setpoint = math::fp32::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<0, 4>())),
            .iq_measured = math::fp32::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<4, 4>()))
        };
        return Ok(self);
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) {
        return os << os.field("iq_setpoint")(self.iq_setpoint) << os.splitter()
            << os.field("iq_measured")(self.iq_measured);
    }
};

//0x17
struct [[nodiscard]] GetBusVoltageCurrent final{
    using Self = GetBusVoltageCurrent;
    static constexpr CommandKind COMMAND = Command::GetBusVoltageCurrent;
    math::fp32 bus_voltage;
    math::fp32 bus_current;
    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self{
            .bus_voltage = math::fp32::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<0, 4>())),
            .bus_current = math::fp32::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<4, 4>()))
        };
        return Ok(self);
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) {
        return os << os.field("bus_voltage")(self.bus_voltage) << os.splitter()
            << os.field("bus_current")(self.bus_current);
    }
};

//0x1c
struct [[nodiscard]] GetTorques final{
    using Self = GetTorques;
    static constexpr CommandKind COMMAND = Command::GetTorques;
    math::fp32 torque_setpoint;
    math::fp32 torque_measured;
    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self{
            .torque_setpoint = math::fp32::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<0, 4>())),
            .torque_measured = math::fp32::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<4, 4>()))
        };
        return Ok(self);
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) {
        return os << os.field("torque_setpoint")(self.torque_setpoint) << os.splitter()
            << os.field("torque_measured")(self.torque_measured);
    }
};

//0x1d
struct [[nodiscard]] GetPowers final{
    using Self = GetPowers;
    static constexpr CommandKind COMMAND = Command::GetPowers;
    math::fp32 eletrical_power;
    math::fp32 mechanical_power;
    static constexpr Result<Self, DeMsgError> try_from_bytes(const std::span<const uint8_t, 8> bytes){
        Self self{
            .eletrical_power = math::fp32::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<0, 4>())),
            .mechanical_power = math::fp32::from_bits(le_bytes_to_int<int32_t>(bytes.subspan<4, 4>()))
        };
        return Ok(self);
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) {
        return os << os.field("eletrical_power")(self.eletrical_power) << os.splitter()
            << os.field("mechanical_power")(self.mechanical_power);
    }
};

}


template<typename T>
static constexpr hal::BxCanFrame serialize_msg_to_can_frame(
    const primitive::AxisId & axis_id, 
    const T & msg
){
    constexpr auto COMMAND = T::COMMAND;
    const FrameId frame_id = FrameId{
        .axis_id = axis_id,
        .command = COMMAND,
    };

    if constexpr(requires { 
        { msg.fill_bytes(std::declval<std::span<uint8_t, 8>>()) } -> std::same_as<void>; 
    }) {
        std::array<uint8_t, 8> bytes;
        msg.fill_bytes(bytes);
        return hal::BxCanFrame::from_parts(
            frame_id.to_stdid(),
            hal::BxCanPayload::from_u8x8(bytes)
        );
    }else{
        return hal::BxCanFrame::from_remote(
            frame_id.to_stdid()
        );
    }
}



}