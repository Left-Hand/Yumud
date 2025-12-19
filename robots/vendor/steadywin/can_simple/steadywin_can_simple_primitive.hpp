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

// 参考代码
//      https://docs.rs/odrive-cansimple/latest/src/odrive_cansimple/enumerations/axis_error.rs.html

namespace ymd::robots::steadywin::can_simple{

namespace primitive{

namespace mit{

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

}

struct [[nodiscard]] AxisId final{
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


struct [[nodiscard]] AxisFaultFlags final{
    using Self = AxisFaultFlags; 
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


static_assert(sizeof(AxisFaultFlags) == 4); 

struct [[nodiscard]] Flags final{ 
    // bit0：电机异常位（odrv0.axis0.motor.error 是否
    // 为 0）
    // bit1：编码器异常位（odrv0.axis0.encoder.error
    // 是否为 0）
    // bit2：控制异常位（odrv0.axis0.controller.error
    // 是否为 0）
    // bit3：系统异常位（odrv0.error 是否为 0）
    // bit7：odrv0.axis0.controller.trajectory_done，即
    // 位置曲线是否执行完毕

    uint8_t motor_error:1;
    uint8_t axis_error:1;
    uint8_t ctrl_error:1;
    uint8_t sys_error:1;

    uint8_t __resv__:3;
    uint8_t trajectory_done:1;
};

static_assert(sizeof(Flags) == 1);  


struct [[nodiscard]] EncoderFaultFlags final{
    using Self = EncoderFaultFlags;
    uint32_t unstable_gain:1;
    uint32_t cpr_out_of_range:1;
    uint32_t no_response:1;
    uint32_t unsupported_encoder_mode:1;
    uint32_t illegal_hall_state:1;
    uint32_t index_not_found_yet:1;
    uint32_t :26;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    } 
};

static_assert(sizeof(EncoderFaultFlags) == 4);

enum class [[nodiscard]] ErrorSource:uint8_t{ 
    Motor = 0,
    Encoder = 1,
    Controller = 3,
    System = 4,
};

static_assert(sizeof(ErrorSource) == 1);

struct [[nodiscard]] FaultFlags final{
    using Self = FaultFlags;
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
    uint32_t :15;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }
};

static_assert(sizeof(FaultFlags) == 4); 

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

static constexpr Option<AxisState> try_into_axis_state(const uint8_t byte){
    if(byte > 11) return None;
    return Some(std::bit_cast<AxisState>(byte));
}

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

static constexpr Option<CommandKind> try_into_command_kind(const uint8_t byte){
    if(byte > 0x1f) return None;
    return Some(std::bit_cast<CommandKind>(byte));
}


enum class [[nodiscard]] LoopMode:uint8_t {
    VoltageLoop = 0,
    CurrentLoop = 1,
    VelocityLoop = 2,
    PositionLoop = 3,
};


static constexpr Option<LoopMode> try_into_loop_mode(const uint8_t byte){
    if(byte > 3) return None;
    return Some(std::bit_cast<LoopMode>(byte));
}


enum class [[nodiscard]] InputMode:uint8_t{
    Inactive = 0,
    PassThrough = 1,
    VelocityRamp = 2,
    PositionFilter = 3,
    MixChannels = 4,
    TrapezoidalTrajectory = 5,
    CurrentRamp = 6,
    Mirror = 7,
};

static constexpr Option<InputMode> try_into_input_mode(const uint8_t byte){
    if(byte > 7) return None;
    return Some(std::bit_cast<InputMode>(byte));
}



struct [[nodiscard]] Command final{
    using Kind = CommandKind;
    constexpr Command(const Kind kind) : kind_(kind){
        if(try_into_command_kind(static_cast<uint8_t>(kind)).is_none()) 
            __builtin_trap();
    }

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
            case Kind::Undefined:   return "Undefined";
            case Kind::Heartbeat:   return "Heartbeat";
            case Kind::Estop:   return "Estop";
            case Kind::GetMotorError:   return "GetMotorError";
            case Kind::RxSdo:   return "RxSdo";
            case Kind::TxSdo:   return "TxSdo";
            case Kind::SetAxisNodeId:   return "SetAxisNodeId";
            case Kind::SetAxisState:    return "SetAxisState";
            case Kind::MitControl:  return "MitControl";
            case Kind::GetEncoderEstimates: return "GetEncoderEstimates";
            case Kind::GetMotorCurrent: return "GetMotorCurrent";
            case Kind::SetControllerMode:   return "SetControllerMode";
            case Kind::SetInputPosition:    return "SetInputPosition";
            case Kind::SetInputVelocity:    return "SetInputVelocity";
            case Kind::SetInputTorque:  return "SetInputTorque";
            case Kind::SetLimits:   return "SetLimits";
            case Kind::StartAnticogging:    return "StartAnticogging";
            case Kind::SetTrajVelLimit: return "SetTrajVelLimit";
            case Kind::SetTrajAccelLimits:  return "SetTrajAccelLimits";
            case Kind::SetTrajInertia:  return "SetTrajInertia";
            case Kind::GetIq:   return "GetIq";
            case Kind::Reboot:  return "Reboot";
            case Kind::GetBusVoltageCurrent:    return "GetBusVoltageCurrent";
            case Kind::ClearErrors: return "ClearErrors";
            case Kind::SetLinearCount:  return "SetLinearCount";
            case Kind::SetPosGain:  return "SetPosGain";
            case Kind::SetVelGain:  return "SetVelGain";
            case Kind::SetTorques:  return "SetTorques";
            case Kind::GetPowers:   return "GetPowers";
            case Kind::DisableCan:  return "DisableCan";
            case Kind::SaveConfig:  return "SaveConfig";
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

struct [[nodiscard]] FrameId final{
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
        return hal::CanStdId::from_u11(
            axis_id.to_b6().connect(command.to_b5()).to_bits()
        );
    }  
};

}

using namespace primitive;

}