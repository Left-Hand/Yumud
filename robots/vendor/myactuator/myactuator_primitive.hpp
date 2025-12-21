#pragma once

#include "core/math/iq/fixed_t.hpp"
#include "core/math/float/fp32.hpp"
#include "core/utils/bytes/bytes_provider.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/string/string_view.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "primitive/arithmetic/percentage.hpp"


namespace ymd::robots::myactuator { 
using namespace ymd::literals;
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

struct [[nodiscard]] SpeedCode_i16{
    int16_t bits;

    constexpr iq8 to_dps() const {
        return bits;
    }
};

struct [[nodiscard]] SpeedLimitCode_u16{
    uint16_t bits;

    static constexpr SpeedLimitCode_u16 from_dps(const uq8 dps){
        return SpeedLimitCode_u16{static_cast<uint16_t>(dps)};
    }

    constexpr uq8 to_dps() const {
        return bits;
    }
};

struct [[nodiscard]] SpeedCtrlCode_i32{
    // 控制值 speedControl 为int32_t类型，对应实际转速为0.01dps/LSB,

    int32_t bits;
    static constexpr SpeedCtrlCode_i32 from_bits(const int32_t bits){
        return SpeedCtrlCode_i32{bits};
    }

    static constexpr  SpeedCtrlCode_i32 from_dps(const iq8 dps){
        return from_bits(static_cast<int32_t>(dps * 100));
    }

    [[nodiscard]] constexpr iq8 to_dps() const {
        return static_cast<iq8>(bits) / 100;
    }
};

struct [[nodiscard]] AccelCode_u32{
    uint32_t bits;

    static constexpr uq16 MAX_DPSS = 60000;
    static constexpr uq16 MIN_DPSS = 100;
    [[nodiscard]] static constexpr Result<AccelCode_u32, std::strong_ordering> 
    try_from_dpss(const uq16 dpss){
        if(dpss > MAX_DPSS) [[unlikely]]
            return Err(std::strong_ordering::greater);
        if(dpss < MIN_DPSS) [[unlikely]]
            return Err(std::strong_ordering::less);

        //1DPSS / LSB
        const uint32_t bits = static_cast<uint32_t>(dpss);
        return Ok(AccelCode_u32{bits});
    }
    [[nodiscard]] constexpr uint32_t to_dpss() const {
        return bits;
    }
};

struct [[nodiscard]] DegreeCode_i16{
    int16_t bits;

    [[nodiscard]] constexpr Angular<iq16> to_angle() const {
        return Angular<iq16>::from_degrees(bits);
    }
};

struct [[nodiscard]] PositionCode_i16{
    int16_t bits;

    constexpr Angular<iq16> to_angle() const {
        return Angular<iq16>::from_degrees(bits);
    }
};


struct [[nodiscard]] LapPosition_u16{
    uint16_t bits;
    constexpr Angular<uq16> to_angle() const {
        return Angular<uq16>::from_turns(uq16::from_bits(bits));
    }
};

struct [[nodiscard]] TemperatureCode_i8{
    int8_t bits;

    [[nodiscard]] int8_t to_celsius() const {
        return bits;
    }
};

struct [[nodiscard]] VoltageCode_u16{
    uint16_t bits;

    [[nodiscard]] uq16 to_volts() const {
        return uq16(0.1) * bits;
    }
};

struct [[nodiscard]] CurrentCode_i16{
    int16_t bits;

    [[nodiscard]] iq16 to_amps() const {
        return iq16(0.01) * bits;
    }
};


struct [[nodiscard]] PositionCode_i32{
    // 0.01degree/LSB
    int32_t bits;

    [[nodiscard]] Angular<iq16> to_angle() const {
        return Angular<iq16>::from_degrees(iq16(0.01) * bits);
    }
};

struct [[nodiscard]] LapPositionCode{
    // 0.01degree/LSB
    uint16_t bits;

    [[nodiscard]] Angular<uq16> to_angle() const {
        return Angular<uq16>::from_degrees(uq16(0.01) * bits);
    }
};

enum class CanAddr:uint8_t{};

struct [[nodiscard]] FaultStatus{
    uint16_t :1;
    uint16_t stall:1;
    uint16_t under_voltage:1;
    uint16_t over_voltage:1;

    uint16_t phase_over_current:1;
    uint16_t :1;
    uint16_t overdrive:1;
    uint16_t param_write:1;

    uint16_t overspeed:1;
    uint16_t :2;
    uint16_t ecs_overheat:1;

    uint16_t motor_overheat:1;
    uint16_t encoder_calibrate_failed:1;
    uint16_t encoder_data_incorrect:1;
    uint16_t :1;

    [[nodiscard]] constexpr uint16_t to_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }

    [[nodiscard]] constexpr bool is_ok() const {
        return to_bits() == 0;
    }
};

enum class PidIndex:uint8_t{
    CurrentKp = 0x01,
    CurrentKi = 0x02,
    SpeedKp = 0x04,
    SpeedKi = 0x05,
    PositionKp = 0x07,
    PositionKi = 0x08,
    PositionKd = 0x09,
};

enum class PlanAccelKind:uint8_t{
    PositionAcc,
    PositionDec,
    SpeedAcc,
    SpeedDec,
};


enum class [[nodiscard]] ReqCommand:uint8_t{
    GetPidParameter = 0x30,
    WritePidParameterToRam = 0x31,
    WritePidParameterToRom = 0x32,
    GetPlanAccel = 0x42,
    SetPlanAccel = 0x43,
    GetMultilapPosition = 0x60,
    GetMultilapPositionWithoutOffset = 0x61,
    GetEncoderMultilapOffset = 0x62,
    WriteEncoderMultilapOffset = 0x62,
    WriteCurrentEncoderMultilapOffset = 0x63,
    ReadLapEncoder = 0x90,
    ReadMultiLapAngle = 0x92,
    ReadLapAngle = 0x94,
    GetStatus1 = 0x9A,
    GetStatus2 = 0x9c,
    GetStatus3 = 0x9d,
    ShutDown = 0x80,
    Stop = 0x81,
    SetTorque = 0xa1, 
    SetSpeed = 0xA2,

    BrakeOn = 0x78,
    BrakeOff = 0x79,
    GetRunMillis = 0xb1,
    GetSwVersion = 0xb2,
    SetOfflineTimeout = 0xb3,
    SetBaudrate = 0xb4,
    GetPackage = 0xb5
};

enum class Baudrate:uint8_t{
    RS485_115200 = 0,
    CAN_500K = 0,
    RS485_500K = 1,
    CAN_1M = 1
};

}
