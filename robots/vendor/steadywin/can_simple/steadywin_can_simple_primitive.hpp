#pragma once

#include <cstdint>
#include "core/stream/ostream.hpp"
#include "core/utils/sumtype.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/math/float/fp32.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/math/real.hpp"

// 伺泰威对Odrive通讯消息进行了魔改 不能直接等效于Odrive
// 参考Odrive的源码，所有Can报文的数据载荷都是8字节
// https://docs.rs/odrive-cansimple/latest/src/odrive_cansimple/enumerations/axis_error.rs.html

namespace ymd::robots::steadywin::can_simple{

namespace primitive{
using namespace ymd::hal;

namespace mit{

#if 1
// p_des:-12.5到 12.5, 单位rad;
// 数据类型为uint16_t, 取值范围为0~65535, 其中0代表-12.5,65535代表 12.5,
//  0~65535中间的所有数值，按比例映射 至-12.5~12.5。
DEF_U16_STRONG_TYPE_GRADATION(MitPositionCode_u16,  from_radians,    
    iq16,   -12.5,  12.5,   25.0/65535)

// v_des:-45到 45, 单位rad/s;
// 数据类型为12位无符号整数，取值范围为0~4095,其中0代表-45,4095代表45,
//  0~4095 中间的所有数值，按比例映射至-45~45。
DEF_U16_STRONG_TYPE_GRADATION(MitSpeedCode_u12,     from_radians,    
    iq16,   -45,    45,     90.0/4095)

// kp: 0到 500;
// 数据类型为12位无符号整数，取值范围为0~4095,其中0代表0,4095代表500,
//  0~4095中间的所有数值，按比例映射至0~500。
DEF_U16_STRONG_TYPE_GRADATION(MitKpCode_u12,        from_val,       
    uq16,   0,      500,    500.0/4095)

// kd: 0到 5;
// 数据类型为12位无符号整数，取值范围为0~4095,其中0代表0, 4095代表5,
//  0~4095中间的所有数值，按比例映射至0~5。
DEF_U16_STRONG_TYPE_GRADATION(MitKdCode_u12,        from_val,       
    uq16,   0,      5,      5.0/4095)

// t_f:-24到 24, 单位N-m;
// 数据类型为12位无符号整数，取值范围为0~4095,其中0代表-24,4095代表24,
//  0~4095中间的所有数值，按比例映射至-24~24。
DEF_U16_STRONG_TYPE_GRADATION(MitTorqueCode_u12,    from_nm,        
    iq16,   -24,      24,     24.0/4095)

#endif
}

struct [[nodiscard]] AxisId{
    using Self = AxisId;

    static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    static constexpr Self from_b6(const literals::Bs6 bs){
        return from_bits(bs.to_bits());
    }

    [[nodiscard]] constexpr uint8_t to_bits() const {
        return bits_;
    }

    constexpr literals::Bs6 to_b6() const {
        return literals::Bs6::from_bits(to_bits());
    }
private:
    uint8_t bits_;
};


struct [[nodiscard]] AxisErrorFlags{
    using Self = AxisErrorFlags; 
    uint32_t invalid_state:1;
    uint32_t under_voltage:1;
    uint32_t over_voltage:1;

    uint32_t current_measurement_timeout:1;
    uint32_t brake_resistor_disarmed:1;
    uint32_t motor_disarmed:1;
    uint32_t motor_failed:1;

    uint32_t sensorless_estimator_failed:1;
    uint32_t encoder_failed:1;
    uint32_t controller_failed:1;
    uint32_t pos_ctrl_during_sensorless:1;

    uint32_t watchdog_timer_expired:1;
    uint32_t min_endstop_pressed:1;
    uint32_t max_endstop_pressed:1;
    uint32_t estop_requested:1;

    uint32_t dc_bus_under_current:1;
    uint32_t dc_bus_over_current:1;
    uint32_t homing_without_endstop:1;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }
};


static_assert(sizeof(AxisErrorFlags) == 4); 

struct Flags{
    uint8_t motor_error:1;
    uint8_t axis_error:1;
    uint8_t ctrl_error:1;
    uint8_t sys_error:1;

    uint8_t __resv__:3;
    uint8_t trajectory_done:1;
};

static_assert(sizeof(Flags) == 1);  


struct [[nodiscard]] EncoderErrorFlags{
    using Self = EncoderErrorFlags;
    uint32_t unstable_gain:1;
    uint32_t cpr_out_of_range:1;
    uint32_t no_response:1;
    uint32_t unsupported_encoder_mode:1;
    uint32_t illegal_hall_state:1;
    uint32_t index_not_found_yet:1;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    } 
};

static_assert(sizeof(EncoderErrorFlags) == 4);

enum class [[nodiscard]] ErrorType:uint8_t{ 
    Motor = 0,
    Encoder = 1,
    Controller = 3,
    System = 4,
};

static_assert(sizeof(ErrorType) == 1);

struct [[nodiscard]] MotorErrorFlags{
    using Self = MotorErrorFlags;
    uint32_t phase_resistance_out_of_range:1;
    uint32_t phase_inductance_out_of_range:1;
    uint32_t adc_failed:1;
    uint32_t drv_fault:1;
    uint32_t control_deadline_missed:1;
    uint32_t not_implemented_motor_type:1;
    uint32_t brake_current_out_of_range:1;
    uint32_t modulation_magnitude:1;
    uint32_t brake_deadtime_violation:1;
    uint32_t unexpected_timer_callback:1;
    uint32_t current_sense_saturation:1;
    uint32_t inverter_over_temp:1;
    uint32_t current_unstable:1;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }
};

static_assert(sizeof(MotorErrorFlags) == 4); 

enum class [[nodiscard]] AxisState:uint8_t{
    Undefined = 0,
    Idle = 1,
    StartupSequence = 2,
    FullCalibrationSequence = 3,
    MotorCalibration = 4,
    SensorlessControl = 5,
    EncoderIndexSearch = 6,
    EncoderOffsetCalibration = 7,
    ClosedLoopControl = 8,
    LockinSpin = 9,
    EncoderDirFind = 10,
    Homing = 11,
};

enum class [[nodiscard]] CommandKind:uint8_t{
    Undefined = 0,
    Heartbeat = 1,
    Estop = 2,
    GetMotorError = 3,
    RxSdo = 4,
    TxSdo = 5,
    SetAxisNodeId = 6,
    SetAxisState = 7,
    MitControl = 8,
    GetEncoderEstimates = 9,
    GetMotorCurrent = 10,
    SetControllerMode = 11,
    SetInputPosition = 12,
    SetInputVelocity = 13,
    SetInputTorque = 14,
    SetLimits = 15,
    StartAnticogging = 16,
    SetTrajVelLimit = 17,
    SetTrajAccelLimits = 18,
    SetTrajInertia = 19,
    GetIq = 20,
    Reboot = 0x16,
    GetBusVoltageCurrent = 0x17,
    ClearErrors = 0x18,
    SetLinearCount = 0x19,
    SetPosGain = 0x1a,
    SetVelGain = 0x1b,
    SetTorques = 0x1c,
    GetPowers = 0x1d,
    DisableCan = 0x1e,
    SaveConfig = 0x1f,
};

enum class [[nodiscard]] ControlMode:uint8_t {
    VoltageControl = 0,
    CurrentControl = 1,
    VelocityControl = 2,
    PositionControl = 3,
};

enum class [[nodiscard]] InputMode:uint8_t{
    Inactive,
    PassThrough,
    VelocityRamp,
    PositionFilter,
    MixChannels,
    TrapezoidalTrajectory,
    CurrentRamp,
    Mirror,
};


namespace details{

}

struct [[nodiscard]] Command{
    using Kind = CommandKind;
    constexpr Command(const Kind kind) : kind_(kind){;}

    static constexpr Command from_b5(literals::Bs5 bs){
        return from_bits(bs.to_bits());
    }

    constexpr literals::Bs5 to_b5() const {
        return literals::Bs5::from_bits_bounded(static_cast<uint8_t>(kind_));
    }

    static constexpr Command from_bits(const uint8_t bits){
        return static_cast<Kind>(bits);
    }

    constexpr CommandKind kind() const{ return kind_; }
    static constexpr const char * err_to_str(const Kind kind){
        switch(kind){
            case Kind::Undefined:
                return "Undefined";
            case Kind::Heartbeat:
                return "Heartbeat";
            case Kind::Estop:
                return "Estop";
            case Kind::GetMotorError:
                return "GetMotorError";
            case Kind::RxSdo:
                return "RxSdo";
            case Kind::TxSdo:
                return "TxSdo";
            case Kind::SetAxisNodeId:
                return "SetAxisNodeId";
            case Kind::SetAxisState:
                return "SetAxisState";
            case Kind::MitControl:
                return "MitControl";
            case Kind::GetEncoderEstimates:
                return "GetEncoderEstimates";
            case Kind::GetMotorCurrent:
                return "GetMotorCurrent";
            case Kind::SetControllerMode:
                return "SetControllerMode";
            case Kind::SetInputPosition:
                return "SetInputPosition";
            case Kind::SetInputVelocity:
                return "SetInputVelocity";
            case Kind::SetInputTorque:
                return "SetInputTorque";
            case Kind::SetLimits:
                return "SetLimits";
            case Kind::StartAnticogging:
                return "StartAnticogging";
            case Kind::SetTrajVelLimit:
                return "SetTrajVelLimit";
            case Kind::SetTrajAccelLimits:
                return "SetTrajAccelLimits";
            case Kind::SetTrajInertia:
                return "SetTrajInertia";
            case Kind::GetIq:
                return "GetIq";
            case Kind::Reboot:
                return "Reboot";
            case Kind::GetBusVoltageCurrent:
                return "GetBusVoltageCurrent";
            case Kind::ClearErrors:
                return "ClearErrors";
            case Kind::SetLinearCount:
                return "SetLinearCount";
            case Kind::SetPosGain:
                return "SetPosGain";
            case Kind::SetVelGain:
                return "SetVelGain";
            case Kind::SetTorques:
                return "SetTorques";
            case Kind::GetPowers:
                return "GetPowers";
            case Kind::DisableCan:
                return "DisableCan";
            case Kind::SaveConfig:
                return "SaveConfig";
        }
        return nullptr;
    }

    using enum Kind;
private:
    CommandKind kind_;

    friend OutputStream & operator<<(OutputStream & os, const CommandKind & kind){ 
        if(const auto * str = err_to_str(kind); str != nullptr)
            return os << str;
        return os << "Custom" << static_cast<uint8_t>(kind);
    } 

    friend OutputStream & operator<<(OutputStream & os, const Command & cmd){ 
        if(const auto * str = err_to_str(cmd.kind()); str != nullptr)
            return os << str;
        return os << "Custom" << static_cast<uint8_t>(cmd.kind());
    }
};

struct [[nodiscard]] FrameId {
    AxisId axis_id;
    Command command;

    static constexpr FrameId from_stdid(const hal::CanStdId & stdid){
        const auto id_u11 = stdid.to_u11();
        return {
            AxisId::from_bits(static_cast<uint8_t>((id_u11 >> 5) & 0b111111)),
            Command::from_bits(id_u11 & 0b11111)
        };
    }

    constexpr hal::CanStdId to_stdid() const { 
        // return hal::CanStdId::from_u11(
        //     static_cast<uint16_t>((axis_id.to_bits() & 0b111111) << 5) | 
        //     (static_cast<uint16_t>(command.kind()) & 0b11111))
        // ;

        return hal::CanStdId::from_u11(
            axis_id.to_b6().connect(command.to_b5()).to_bits()
        );
    }  
};

}


using namespace primitive;

namespace msgs{
struct [[nodiscard]] Heartbeat{
    using Self = Heartbeat;
    static constexpr CommandKind command =  Command::Heartbeat;
    AxisErrorFlags axis_error;
    AxisState axis_state;

    // bit0：电机异常位（odrv0.axis0.motor.error 是否
    // 为 0）
    // bit1：编码器异常位（odrv0.axis0.encoder.error
    // 是否为 0）
    // bit2：控制异常位（odrv0.axis0.controller.error
    // 是否为 0）
    // bit3：系统异常位（odrv0.error 是否为 0）
    // bit7：odrv0.axis0.controller.trajectory_done，即
    // 位置曲线是否执行完毕
    Flags flags;
    uint8_t __resv__;

    // 周期消息的生命值，每一个⼼跳消息加 1，范围
    // 0-255，如果此生命值不连续，表示⼼跳消息丢
    // 失，即通信不稳
    uint8_t life;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

static_assert(sizeof(Heartbeat) == 8);

// CMD ID: 0x002（主机→电机）⽆参数⽆数据。
// 此指令会导致电机紧急停机，并报 ESTOP_REQUESTED 异常
struct [[nodiscard]] Estop{
    static constexpr CommandKind command = Command::Estop;
};

struct [[nodiscard]] GetError{
    static constexpr CommandKind command = Command::GetMotorError;
};

// CMD ID: 0x003（电机→主机）
struct [[nodiscard]] GetErrorReq{
    using Self = GetErrorReq;
    static constexpr CommandKind command = Command::GetMotorError;
    ErrorType type;
};

// CMD ID: 0x003（电机←主机）
struct [[nodiscard]] GetErrorResp{
    using Self = GetErrorResp;
    static constexpr CommandKind command = Command::GetMotorError;
    union{
        uint64_t motor_exception;
        uint32_t encoder_exception;
        uint32_t controller_exception;
        uint32_t system_exception;
    };
    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }
};

static_assert(sizeof(GetErrorResp) == 8);   


// struct [[nodiscard]] EncoderCount{
//     static constexpr CommandKind command = Command::GetEncoderCount;
//     int32_t shadow_count;
//     int32_t cpr_count;
// };
struct [[nodiscard]] RxSdo{
    using Self = RxSdo;
    static constexpr CommandKind command = Command::RxSdo;
    bool is_read;
    uint32_t endpoint_id;
    uint8_t __resv__;
    uint32_t value;
};

struct [[nodiscard]] TxSdo{
    using Self = RxSdo;
    static constexpr CommandKind command = Command::TxSdo;
    bool is_read;
    uint32_t endpoint_id;
    uint8_t __resv__;
    uint32_t value;
};

// CMD ID: 0x006（主机→电机）
struct [[nodiscard]] SetAxisNodeId{
    using Self = SetAxisNodeId;
    static constexpr CommandKind command = Command::SetAxisNodeId;
    uint32_t node_id;
};


// CMD ID: 0x007（主机→电机）
struct [[nodiscard]] SetAxisState{
    using Self = SetAxisState;
    static constexpr CommandKind command = Command::SetAxisState;
    AxisState state;
};


struct [[nodiscard]] MitControl{
    
    using Self = MitControl;
    static constexpr CommandKind command = Command::MitControl;

    mit::MitPositionCode_u16 position;
    mit::MitSpeedCode_u12 speed;
    mit::MitKpCode_u12 kp;
    mit::MitKdCode_u12 kd;
    mit::MitTorqueCode_u12 torque;

    constexpr hal::CanClassicPayload to_payload(){
        std::array<uint8_t, 8> bytes;
        bytes[0] = static_cast<uint8_t>(position.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(position.to_bits() & 0xff);
        bytes[2] = static_cast<uint8_t>(speed.to_bits() >> 4);
        bytes[3] = static_cast<uint8_t>(((speed.to_bits() & 0xf) << 4) | ((kp.to_bits() >> 8)));
        bytes[4] = static_cast<uint8_t>(kp.to_bits() & 0xff);
        bytes[5] = static_cast<uint8_t>(kd.to_bits() >> 4);
        bytes[6] = static_cast<uint8_t>(((kd.to_bits() & 0xf) << 4) | ((torque.to_bits() >> 8)));
        bytes[7] = static_cast<uint8_t>(torque.to_bits() & 0xf);
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(bytes));
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
    static constexpr CommandKind command = Command::GetEncoderEstimates;
    math::fp32 position;
    math::fp32 velocity;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

//ID 0x00A
struct [[nodiscard]] GetMotorCurrent{
    using Self = GetMotorCurrent;
    static constexpr CommandKind command = Command::GetMotorCurrent;
    int32_t shadow_count;
    int32_t count_n_cpr;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

//ID 0x00B
struct [[nodiscard]] SetCotrollerMode{
    using Self = SetCotrollerMode;
    static constexpr CommandKind command = Command::SetControllerMode;
    ControlMode control_mode;
    InputMode input_mode;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return Self {
            .control_mode = std::bit_cast<ControlMode>(payload[0]),
            .input_mode = std::bit_cast<InputMode>(payload[4])
        };
    }

    constexpr hal::CanClassicPayload to_payload() const {
        //stupid padding
        std::array<uint8_t, 8> bytes = {
            std::bit_cast<uint8_t>(control_mode),
            0, 0, 0,
            std::bit_cast<uint8_t>(input_mode),
            0, 0, 0
        };
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(bytes));
    }
};

//ID 0x00C
struct [[nodiscard]] SetInputPosition{ 
    using Self = SetInputPosition;
    static constexpr auto command = CommandKind{0x00c};

    math::fp32 input_position;
    int16_t vel_ff;
    int16_t torque_ff;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetInputVelocity{ 
    using Self = SetInputVelocity;
    static constexpr auto command = CommandKind{0x00d};

    math::fp32 vel_ff;
    math::fp32 torque_ff;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetInputTorque{ 
    using Self = SetInputTorque;
    static constexpr auto command = CommandKind{0x00e};

    math::fp32 torque_ff;
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};


struct [[nodiscard]] SetLimits{
    using Self = SetLimits;
    static constexpr auto command = CommandKind{0x00f};

    math::fp32 velocity_limit;
    math::fp32 current_limit;
    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] StartAntiCogging{
    using Self = StartAntiCogging;
    static constexpr auto command = CommandKind{0x010};
};

struct [[nodiscard]] SetTrajVelLimit{
    using Self = SetTrajVelLimit;
    static constexpr auto command = CommandKind{0x011};

    math::fp32 traj_vel_limit;
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetTrajAccelLimit{ 
    using Self = SetTrajAccelLimit;
    static constexpr auto command = CommandKind{0x012};

    math::fp32 traj_accel_limit;
    math::fp32 traj_decel_limit;
    

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetTrajInertia{
    using Self = SetTrajInertia;
    static constexpr auto command = CommandKind{0x013};
    math::fp32 traj_inertia;//惯量
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] GetIq{ 
    using Self = GetIq;
    static constexpr auto command = CommandKind{0x014};
    math::fp32 id_setpoint;
    math::fp32 iq_measured;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] Reboot{ 
    using Self = Reboot;
    static constexpr auto command = CommandKind{0x016};
};

struct [[nodiscard]] GetBusVoltageCurrent{
    using Self = GetBusVoltageCurrent;
    static constexpr auto command = CommandKind{0x017};
    math::fp32 bus_voltage;
    math::fp32 bus_current;
    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};


struct [[nodiscard]] ClearErrors{ 
    using Self = ClearErrors;
    static constexpr auto command = CommandKind{0x018};
};

struct [[nodiscard]] SetLinearCount{
    using Self = SetLinearCount;
    static constexpr auto command = CommandKind{0x019};
    int32_t linear_count;
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};


struct [[nodiscard]] SetPosGain{
    using Self = SetPosGain;
    static constexpr auto command = CommandKind{0x01a};
    math::fp32 pos_gain;
    uint32_t __padding__ = 0;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};


struct [[nodiscard]] SetVelGain{
    using Self = SetVelGain;
    static constexpr auto command = CommandKind{0x01b};
    math::fp32 vel_gain;
    math::fp32 vel_integrator_gain;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] SetTorques{
    using Self = SetTorques;
    static constexpr auto command = CommandKind{0x01c};
    math::fp32 torque_setpoint;
    math::fp32 torque;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] GetPowers{
    using Self = GetPowers;
    static constexpr auto command = CommandKind{0x01d};
    math::fp32 electrical_power;
    math::fp32 mechanical_power;

    static constexpr Self form_payload(const hal::CanClassicPayload & payload){
        return std::bit_cast<Self>(payload.u8x8());
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

struct [[nodiscard]] DisableCan{
    using Self = DisableCan;
    static constexpr auto command = CommandKind{0x01e};
};

struct [[nodiscard]] SaveConfig{
    using Self = SaveConfig;
    static constexpr auto command = CommandKind{0x01f};
};



}

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




}