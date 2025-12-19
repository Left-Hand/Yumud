#pragma once

#include "myactuator.hpp"

namespace ymd::robots::myactuator{
namespace req_msgs{

#define DEF_COMMAND_ONLY_REQ_MSG(cmd_t)\
struct [[nodiscard]] cmd_t final{\
    constexpr \
    void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {\
    BytesFiller(bytes).fill_remaining(0);}};\

struct [[nodiscard]] GetPidParameter{
    PidIndex index;
    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_byte(std::bit_cast<uint8_t>(index));
        filler.fill_remaining(0);
    }
};


struct [[nodiscard]] GetPlanAccel final{
    PlanAccelKind kind;
    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_byte(std::bit_cast<uint8_t>(kind));
        filler.fill_remaining(0);
    }
};

struct [[nodiscard]] SetPlanAccel final{
    PlanAccelKind kind;
    AccelCode_u32 accel;
    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_byte(std::bit_cast<uint8_t>(kind));
        filler.push_zeros(2);
        filler.push_int<uint32_t>(accel.bits);
    }
};


DEF_COMMAND_ONLY_REQ_MSG(GetStatus1);
DEF_COMMAND_ONLY_REQ_MSG(GetStatus2);
DEF_COMMAND_ONLY_REQ_MSG(GetStatus3);
DEF_COMMAND_ONLY_REQ_MSG(ShutDown);

struct [[nodiscard]] SetTorque final{
    CurrentCode_i16 q_current;
    constexpr 
    void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_zeros(3);
        filler.push_int<int16_t>(q_current.bits);
        filler.push_zeros(2);
    };
};


struct [[nodiscard]] SetSpeed final{
    Percentage<uint8_t> rated_current_ratio;
    SpeedCtrlCode_i32 speed;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_int<uint8_t>(rated_current_ratio.percents());
        filler.push_zeros(2);
        filler.push_int<int32_t>(speed.bits);
    };
};



struct [[nodiscard]] SetPosition final{
    SpeedLimitCode_u16 speed_limit;
    PositionCode_i32 abs_position;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_zeros(1);
        filler.push_int<uint16_t>(speed_limit.bits);
        filler.push_int<int32_t>(abs_position.bits);
    };
};


struct [[nodiscard]] SetLapPosition final{
    // 1.角度控制值angleControl 为 uint16_t 类型， 数值范围0~35999, 对应实际位置为
    // 0.01degree/LSB, 即实际角度范围0°~359.99°;
    //  2.spinDirection设置电机转动的方向， 为 uint8_t类型，0x00代表顺时针，0x01代表
    // 逆时针；
    // 3.maxSpeed限制了电机转动的最大速度，为uint16_t类型，对应实际转速1dps/LSB。
    LapPosition_u16 lap_position;
    bool is_ccw;
    SpeedLimitCode_u16 max_speed;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_int<uint8_t>(is_ccw);
        filler.push_int<uint16_t>(max_speed.bits);
        filler.push_int<uint16_t>(lap_position.bits);
        filler.push_zeros(2);
    };
};

// 该指令为控制指令，在电机没有故障的情况下可以运行该指令。主机发送该命令以
// 控制电机的位置(多圈角度), 若给定的电流大于堵转电流， 则不开启力控模式，
// 电机的最大转矩电流由上位机中的电机堵转电流值限制。
struct [[nodiscard]] SetTorquePosition final{
    // 控制值maxTorque限制了电机输出轴的最大扭矩，为uint8_t
    // 类型，取值范围为0~255, 以额定电流的百分比为单位， 即 1%*额定电流LSB
    Percentage<uint8_t> rated_current_ratio;

    // 控制值maxSpeed限制了电机输出轴转动的最大速度，为uint16t类型，
    // 对应实际转速1dps/LSB
    SpeedLimitCode_u16 max_speed;
    
    // 控制值angleControl为 int32t类型， 对应实际位置为0.01degree/LSB, 
    // 即 36000代表360° ,电机转动方向由目标位置和当前位置的差值决定
    PositionCode_i32 position;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_int<uint8_t>(rated_current_ratio.percents());
        filler.push_int<uint16_t>(max_speed.bits);
        filler.push_int<int32_t>(position.bits);
    };
};


struct [[nodiscard]] MitParams final{
    MitPositionCode_u16 position;
    MitSpeedCode_u12 speed;
    MitKpCode_u12 kp;
    MitKdCode_u12 kd;
    MitTorqueCode_u12 torque;

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
};


#undef DEF_COMMAND_ONLY_REQ_MSG
};


enum class [[nodiscard]] LoopWiring:uint8_t{
    Current,
    Speed,
    Position
};


namespace resp_msgs{

#define DEF_COMMAND_ONLY_RESP_MSG(cmd_t)\
struct [[nodiscard]] cmd_t final{};
// struct cmd{
//    constexpr CommandHeadedDataFielfill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
//        return CommandHeadedDataField::from_command(Command::cmd)};}

struct [[nodiscard]] GetPidParameter final{
    using Self = GetPidParameter;
    PidIndex index;
    math::fp32 value;

    [[nodiscard]] static constexpr Result<Self, DeMsgError>
    try_from_bytes(const std::span<const uint8_t, 7> bytes){
        return Ok(Self{
            .index = std::bit_cast<PidIndex>(bytes[0]),
            .value = le_bytes_ctor_bits(bytes.subspan<3, 4>())
        });
    }
};


struct [[nodiscard]] GetPlanAccel final{
    using Self = GetPlanAccel;
    PlanAccelKind kind;
    AccelCode_u32 accel;
    [[nodiscard]] static constexpr Result<Self, DeMsgError>
    try_from_bytes(const std::span<const uint8_t, 7> bytes){
        return Ok(Self{
            .kind = std::bit_cast<PlanAccelKind>(bytes[0]), 
            .accel = le_bytes_ctor_bits(bytes.subspan<3, 4>())
        });
    }
};


DEF_COMMAND_ONLY_RESP_MSG(GetStatus1);

DEF_COMMAND_ONLY_RESP_MSG(GetStatus2);

struct [[nodiscard]] GetStatus3 final{
    TemperatureCode_i8 motor_temperature;

    struct {
        CurrentCode_i16 a;
        CurrentCode_i16 b;
        CurrentCode_i16 c;
    }phase_current;
};

// 1.电机温度temperature (int8t类型，1°C/lsb
// 2.电机的转矩电流值iq (int16_t类型， 0.01A/lsb
// 3.电机输出轴转速speed (int16t类型， 1dps/lsb
// 4.电机输出轴角度 (intl6t类型，1degree/LSB,最大范围±32767degree)。
template<typename Derived>
struct [[nodiscard]] _MotorStatusReport{
    using Self = Derived;
    TemperatureCode_i8 motor_temperature;
    CurrentCode_i16 q_current;
    SpeedCode_i16 axis_speed;
    DegreeCode_i16 axis_degrees;

    [[nodiscard]] static constexpr Result<Derived, DeMsgError> 
    try_from_bytes(const std::span<const uint8_t, 7> bytes){
        Derived ret;
        const auto exacter = make_bytes_ctor_bits_exacter<std::endian::little>(bytes);
        exacter.exact_to_elements(ret.motor_temperature, ret.q_current, ret.axis_speed, ret.axis_degrees);
        return Ok(ret);
    };
};

// 1.电机温度temperature (int8t类型，1°C/lsb
//  2.电机的转矩电流值iq (int16_t类型， 0.01A/lsb
//  3.电机输出轴转速speed (int16t类型， 1dps/lsb
//  4.编码器位置值encoder(uint16 t类型，编码器的数值范围由编码器位数决定)
template<typename Derived>
struct [[nodiscard]] _MotorStatusReport2{
    using Self = Derived;
    TemperatureCode_i8 motor_temperature;
    CurrentCode_i16 q_current;
    SpeedCode_i16 axis_speed;
    LapPosition_u16 axis_lap_position;

    static constexpr Result<Self, DeMsgError>
    try_from_bytes(const std::span<const uint8_t, 7> bytes){
        Self ret;
        const auto exacter = make_bytes_ctor_bits_exacter<std::endian::little>(bytes);
        exacter.exact_to_elements(ret.motor_temperature, ret.q_current, ret.axis_speed, ret.axis_lap_position);
        return ret;
    };
};

// 1.电机温度temperature (int8t类型，1°C/lsb
//  2.电机的转矩电流值iq (int16_t类型， 0.01A/lsb
//  3.电机输出轴转速speed (int16t类型， 1dps/lsb
//  4.电机输出轴角度 (intl6t类型，1degree/LSB,最大范围±32767degree)。

struct [[nodiscard]] SetTorque final:public _MotorStatusReport<SetTorque>{};
// 1.电机温度temperature (int8t类型， 1°C/lsb
//  2.电机的转矩电流值iq (intl6_t类型， 0.01A/lsb
//  3.电机输出轴转速speed (int16t类型， 1dps/lsb
//  4.电机输出轴角度 (intl6_t类型，1degree/LSB,最大范围±32767degree)。
struct [[nodiscard]] SetSpeed final:public _MotorStatusReport<SetSpeed>{};

// 1.电机温度temperature (int8t类型， 1°C/lsb
//  2.电机的转矩电流值iq(int16t类型， 0.01A/lsb
//  3.电机输出轴转速speed (int16_t类型， 1dps/lsb
//  4.电机输出轴角度(intl6t类型，Idegree/LSB,最大范围±32767degree)。
struct [[nodiscard]] SetPosition final:public _MotorStatusReport<SetPosition>{};

// 1.电机温度temperature (int8t类型，1°C/lsb
//  2.电机的转矩电流值iq (int16_t类型， 0.01A/lsb
//  3.电机输出轴转速speed (int16t类型， 1dps/lsb
//  4.编码器位置值encoder(uint16 t类型，编码器的数值范围由编码器位数决定)
struct [[nodiscard]] SetLapPosition final:public _MotorStatusReport2<SetLapPosition>{};

// 1.电机温度temperature (int8t类型， 1℃/lsb
//  2.电机的转矩电流值iq(int16_t类型， 0.01A/lsb
//  3.电机输出轴转速speed (int16_t类型， 1dps/lsb
//  4.电机输出轴角度 (intl6_t类型，1degree/LSB,最大范围±32767degree)。
struct [[nodiscard]] DeltaPosition final:public _MotorStatusReport<DeltaPosition>{};

// 1.电机温度temperature (int8t类型， 1℃/lsb
//  2.电机的转矩电流值iq(int16_t类型， 0.01A/lsb
//  3.电机输出轴转速speed (int16_t类型， 1dps/lsb
//  4.电机输出轴角度 (intl6_t类型，1degree/LSB,最大范围±32767degree)。
struct [[nodiscard]] SetTorquePosition final:public _MotorStatusReport<SetTorquePosition>{};


struct [[nodiscard]] GetPackage final{
    using Self = GetPackage;
    static constexpr size_t MAX_STR_LENGTH = PAYLOAD_CAPACITY;
    char str[MAX_STR_LENGTH];

    static constexpr Result<Self, DeMsgError>
    try_from_bytes(const std::span<const uint8_t, PAYLOAD_CAPACITY> bytes){
        Self self;
        for(size_t i = 0; i < MAX_STR_LENGTH; i++){
            self.str[i] = bytes[i];
        }
        return Ok(self);
    }

    friend OutputStream& operator<<(OutputStream & os, const Self & self){ 
        return os << StringView(self.str, MAX_STR_LENGTH);
    }
};

DEF_COMMAND_ONLY_RESP_MSG(ShutDown);


struct [[nodiscard]] MitParams final{
    using Self = MitParams;
    CanAddr can_addr;
    MitPositionCode_u16 position;
    MitSpeedCode_u12 speed;
    MitTorqueCode_u12 torque;

    constexpr Result<Self, DeMsgError> try_from_bytes(std::span<const uint8_t, 8> bytes) const {
        const uint8_t can_addr_bits = 
            bytes[0];
        const uint16_t position_bits = 
            (bytes[1] << 8) | bytes[2];
        const uint16_t speed_bits = 
            (bytes[3] << 4) | (bytes[4] >> 4);
        const uint16_t torque_bits = 
            ((bytes[4] & 0x0f) << 8) | (bytes[5]);
        return Ok(Self{
            .can_addr = CanAddr(can_addr_bits),
            .position = MitPositionCode_u16::from_bits(position_bits),
            .speed = MitSpeedCode_u12::from_bits(speed_bits),
            .torque = MitTorqueCode_u12::from_bits(torque_bits)
        });
    };
};


};

}