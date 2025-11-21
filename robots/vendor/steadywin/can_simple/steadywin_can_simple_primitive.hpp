#pragma once

#include <cstdint>
#include "core/stream/ostream.hpp"
#include "core/utils/sumtype.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/math/float/fp32.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/math/iq/fixed_t.hpp"

// 伺泰威对Odrive通讯消息进行了魔改 不能直接等效于Odrive
// 参考Odrive的源码，所有Can报文的数据载荷都是8字节
// https://docs.rs/odrive-cansimple/latest/src/odrive_cansimple/enumerations/axis_error.rs.html

namespace ymd::robots::steadywin::can_simple{

namespace primitive{
using namespace ymd::hal;

namespace mit{

#if 0
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
};

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
    Undefined,
    OdriveHeartbeat = 1,
    OdriveEstop,
    GetMotorError,
    GetEncoderError,
    GetSensorlessError,
    SetAxisNodeId,
    SetAxisRequestedState,
    SetAxisStartupConfig,
    GetEncoderEstimates,
    GetEncoderCount,
    SetControllerModes,
    SetInputPos,
    SetInputVel,
    SetInputCurrent,
    SetVelLimit,
    StartAnticogging,
    SetTrajVelLimit,
    SetTrajAccelLimits,
    SetTrajAPerCss,
    GetIq,
    GetSensorlessEstimates,
    ResetOdrive,
    GetVbusVoltage,
    ClearErrors,
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
            case Kind::Undefined:return "Undefined";
            case Kind::OdriveHeartbeat:return "OdriveHeartbeat";
            case Kind::OdriveEstop:return "OdriveEstop";
            case Kind::GetMotorError:return "GetMotorError";
            case Kind::GetEncoderError:return "GetEncoderError";
            case Kind::GetSensorlessError:return "GetSensorlessError";
            case Kind::SetAxisNodeId:return "SetAxisNodeId";
            case Kind::SetAxisRequestedState:return "SetAxisRequestedState";
            case Kind::SetAxisStartupConfig:return "SetAxisStartupConfig";
            case Kind::GetEncoderEstimates:return "GetEncoderEstimates";
            case Kind::GetEncoderCount:return "GetEncoderCount";
            case Kind::SetControllerModes:return "SetControllerModes";
            case Kind::SetInputPos:return "SetInputPos";
            case Kind::SetInputVel:return "SetInputVel";
            case Kind::SetInputCurrent:return "SetInputCurrent";
            case Kind::SetVelLimit:return "SetVelLimit";
            case Kind::StartAnticogging:return "StartAnticogging";
            case Kind::SetTrajVelLimit:return "SetTrajVelLimit";
            case Kind::SetTrajAccelLimits:return "SetTrajAccelLimits";
            case Kind::SetTrajAPerCss:return "SetTrajAPerCss";
            case Kind::GetIq:return "GetIq";
            case Kind::GetSensorlessEstimates:return "GetSensorlessEstimates";
            case Kind::ResetOdrive:return "ResetOdrive";
            case Kind::GetVbusVoltage:return "GetVbusVoltage";
            case Kind::ClearErrors:return "ClearErrors";
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
    static constexpr CommandKind command =  Command::OdriveHeartbeat;
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

    static constexpr Self from_u8x8(const std::array<uint8_t, 8> bytes){
        return std::bit_cast<Self>(bytes);;
    }

    constexpr hal::CanClassicPayload to_payload() const {
        return hal::CanClassicPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

static_assert(sizeof(Heartbeat) == 8);

struct [[nodiscard]] Estop{
    static constexpr CommandKind command = Command::OdriveEstop;
};

struct [[nodiscard]] GetError{
};

struct [[nodiscard]] GetErrorReq{
    using Self = GetErrorReq;
    static constexpr CommandKind command = Command::GetMotorError;
    ErrorType type;
};



struct [[nodiscard]] GetErrorResp{
    using Self = GetErrorResp;
    static constexpr CommandKind command = Command::GetMotorError;
    Flags flags;
};

struct [[nodiscard]] EncoderCount{
    int32_t shadow_count;
    int32_t cpr_count;
};

struct [[nodiscard]] EncoderEstimates{
    fp32 position;
    fp32 velocity;
};



struct [[nodiscard]] MotorCurrent{
    fp32 setpoint;
    fp32 measurement;
};


struct [[nodiscard]] VbusVoltage{
    fp32 voltage;
};

using EncoderErrorFlags = primitive::EncoderErrorFlags;
using MotorErrorFlags = primitive::MotorErrorFlags;
}

struct [[nodiscard]] Msg:public Sumtype<
    msgs::EncoderCount,
    msgs::EncoderEstimates,
    msgs::Heartbeat,
    msgs::MotorCurrent,
    msgs::VbusVoltage,
    EncoderErrorFlags,
    MotorErrorFlags
>{
    using EncoderCount = msgs::EncoderCount;
    using EncoderEstimates = msgs::EncoderEstimates;
    using Heartbeat = msgs::Heartbeat;
    using MotorCurrent = msgs::MotorCurrent;
    using VbusVoltage = msgs::VbusVoltage;
    using EncoderErrorFlags = msgs::EncoderErrorFlags;
    using MotorErrorFlags = msgs::MotorErrorFlags;
};

struct [[nodiscard]] Event{
    AxisId axis_id;
    Msg signal;
};

struct [[nodiscard]] FrameSerializer{
    using Error = Infallible;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using CanResult = Result<hal::CanClassicFrame, Error>;

    static constexpr CanResult get_motor_error(AxisId axis_id){
        return request(axis_id, Command::GetMotorError);
    }

    static constexpr CanResult get_encoder_error(AxisId axis_id){
        return request(axis_id, Command::GetEncoderError);
    }

    static constexpr CanResult get_sensorless_error(AxisId axis_id){
        return request(axis_id, Command::GetSensorlessError);
    }

    static constexpr CanResult get_encoder_estimates(AxisId axis_id){
        return request(axis_id, Command::GetEncoderEstimates);
    }

    static constexpr CanResult get_encoder_count(AxisId axis_id){
        return request(axis_id, Command::GetEncoderCount);
    }

    static constexpr CanResult get_iq(AxisId axis_id){
        return request(axis_id, Command::GetIq);
    }

    static constexpr CanResult get_sensorless_estimate(AxisId axis_id){
        return request(axis_id, Command::GetSensorlessEstimates);
    }

    static constexpr CanResult get_vbus_voltage(AxisId axis_id){
        return request(axis_id, Command::GetVbusVoltage);
    }

    static constexpr CanResult request(AxisId axis_id, Command cmd){ 
        return Ok(hal::CanClassicFrame::from_remote(encode_id(axis_id, cmd)));
    }


    static constexpr CanResult emergency_stop(AxisId id) {
        return make_msg(FrameId{id, Command::OdriveEstop});
    }

    static constexpr CanResult reboot(AxisId id) {
        return make_msg(FrameId{id, Command::ResetOdrive});
    }


    static constexpr CanResult clear_errors(AxisId id) {
        return make_msg(FrameId{id, Command::ClearErrors});
    }


    static constexpr CanResult start_anticogging(AxisId id) {
        return make_msg(FrameId{id, Command::StartAnticogging});
    }


    static constexpr CanResult set_axis_node_id(AxisId id, AxisId new_id) {
        return make_msg(
            FrameId{id, Command::SetAxisNodeId}, 
            static_cast<uint32_t>(new_id.to_bits())
        );
    }


    static constexpr CanResult set_axis_requested_state(AxisId id, AxisState axis_state) {
        return make_msg(
            FrameId{id, Command::SetAxisRequestedState}, 
            static_cast<uint32_t>(axis_state)
        );
    }

    static constexpr CanResult set_input_current(AxisId id, float value) {
        const auto scaled_value = static_cast<uint32_t>(static_cast<int32_t>(
            100.0f * value + 0.5f)
        );
        return make_msg(
            FrameId{id, Command::SetInputCurrent}, 
            scaled_value
        );
    }


    static constexpr CanResult set_velocity_limit(AxisId id, fp32 value) {
        const auto value_bits = std::bit_cast<uint32_t>(value);
        return make_msg(
            FrameId{id, Command::SetVelLimit}, 
            value_bits
        );
    }

    static constexpr CanResult set_controller_modes(AxisId id, ControlMode control_mode) {
        return make_msg(
            FrameId{id, Command::SetControllerModes}, 
            static_cast<uint32_t>(control_mode),
            static_cast<uint32_t>(InputMode::PassThrough)
        );
    }

// private:
    static constexpr hal::CanStdId encode_id(const AxisId axis_id, const Command cmd){
        return FrameId{axis_id, cmd}.to_stdid();
    }

    static constexpr CanResult make_msg(FrameId frame_id){
        return Ok(hal::CanClassicFrame::from_empty(frame_id.to_stdid()));
    }

    static constexpr CanResult make_msg(FrameId frame_id, const uint32_t arg1){
        return Ok(
            hal::CanClassicFrame::from_parts(
                frame_id.to_stdid(),
                hal::CanClassicPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(arg1))
            )
        );
    }

    static constexpr CanResult make_msg(FrameId frame_id, const uint32_t arg1, const uint32_t arg2){
        std::array<uint8_t, 8> bytes;
        const auto arr1 = std::bit_cast<std::array<uint8_t, 4>>(arg1);
        const auto arr2 = std::bit_cast<std::array<uint8_t, 4>>(arg2);
        std::copy(arr1.begin(), arr1.end(), bytes.begin());
        std::copy(arr2.begin(), arr2.end(), bytes.begin() + 4);
        return Ok(
            hal::CanClassicFrame::from_parts(
                frame_id.to_stdid(), 
                hal::CanClassicPayload::from_bytes(bytes)
            )
        );
    }

};

struct [[nodiscard]] FrameDeserializer{
    enum class [[nodiscard]] Error:uint8_t{
        FrameIsNotStd,
        PayloadTooShort,
        NotImplemented
    };

private:
    struct [[nodiscard]] BytesReader{
        explicit constexpr BytesReader(std::span<const uint8_t> bytes) : 
            bytes_(bytes) {}

        [[nodiscard]] constexpr Option<int32_t> fetch_i32(){
            if(remaining().size() < 4)
                return None;
            return Some(le_bytes_to_int<int32_t>(fetch_bytes<4>()));
        }
        
        [[nodiscard]] constexpr Option<uint32_t> fetch_u32(){
            if(remaining().size() < 4)
                return None;
            return Some(le_bytes_to_int<uint32_t>(fetch_bytes<4>()));
        }


        [[nodiscard]] constexpr Option<fp32> fetch_f32(){
            if(remaining().size() < 4)
                return None;
            return Some(fp32::from_bits(le_bytes_to_int<int32_t>(fetch_bytes<4>())));
        }
    private:
        std::span<const uint8_t> bytes_;

        template<size_t N>
        [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_bytes(){
            const auto ret = std::span<const uint8_t, N>(bytes_.data(), N);
            bytes_ = std::span<const uint8_t>(bytes_.data() + N, bytes_.size() - N);
            return ret;
        }

        [[nodiscard]] constexpr std::span<const uint8_t> remaining() const {
            return bytes_;
        }
    };
public:
    static constexpr auto map_frame_to_event(const hal::CanClassicFrame & frame) -> Result<Event, Error> {
        if(not frame.is_standard())
            return Err(Error::FrameIsNotStd);

        const auto frame_id = FrameId::from_stdid(frame.identifier().to_stdid());
        const auto axis_id = frame_id.axis_id;
        const auto command = frame_id.command;
        BytesReader reader(frame.payload_bytes());

        #define UNWRAP_PAYLOAD(expr) \
        ({\
            const auto may = (expr);\
            if(may.is_none())\
                return Err(Error::PayloadTooShort);\
            may.unwrap();\
        });\

        switch(command.kind()){
            case Command::GetEncoderCount:{
                const auto shadow_count = UNWRAP_PAYLOAD(reader.fetch_i32());
                const auto cpr_count = UNWRAP_PAYLOAD(reader.fetch_i32());
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::EncoderCount{
                        shadow_count: shadow_count,
                        cpr_count: cpr_count,
                    }
                });
            }
            
            case Command::GetEncoderError:{
                const auto error_bits = UNWRAP_PAYLOAD(reader.fetch_u32());
                // Note: In your C++ code there's no EncoderErrorFlags::from_bits method shown,
                // so assuming direct bit interpretation or you'll need to implement that conversion
                const EncoderErrorFlags axis_error = EncoderErrorFlags::from_bits(error_bits);
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::EncoderErrorFlags{axis_error}
                });
            }
            
            case Command::GetEncoderEstimates:{
                const auto position = UNWRAP_PAYLOAD(reader.fetch_f32());
                const auto velocity = UNWRAP_PAYLOAD(reader.fetch_f32());
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::EncoderEstimates{
                        position: position,
                        velocity: velocity,
                    }
                });
            }
            
            case Command::OdriveHeartbeat:{
                return Ok(
                    Event{
                        .axis_id = axis_id,
                        .signal = msgs::Heartbeat::from_u8x8(frame.payload().u8x8())
                    }
                );
            }
            
            case Command::GetIq:{
                const auto setpoint = UNWRAP_PAYLOAD(reader.fetch_f32());
                const auto measurement = UNWRAP_PAYLOAD(reader.fetch_f32());
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::MotorCurrent{
                        setpoint: setpoint,
                        measurement: measurement,
                    }
                });
            }
            
            case Command::GetMotorError:{
                const auto error_bits = UNWRAP_PAYLOAD(reader.fetch_u32());
                // Similar bit mapping for MotorErrorFlags
                const MotorErrorFlags axis_error = MotorErrorFlags::from_bits(error_bits);
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::MotorErrorFlags{axis_error}
                });
            }
            
            case Command::GetVbusVoltage:{
                const auto voltage = UNWRAP_PAYLOAD(reader.fetch_f32());
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::VbusVoltage{
                        voltage: voltage,
                    }
                });
            }
            default:
                return Err(Error::NotImplemented);
        }

        #undef UNWRAP_PAYLOAD
    }

};


}