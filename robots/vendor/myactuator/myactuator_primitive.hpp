#pragma once

#include "core/math/iq/fixed_t.hpp"
#include "core/math/float/fp32.hpp"
#include "core/utils/bytes/bytes_provider.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/string/string_view.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "primitive/arithmetic/percentage.hpp"
#include "robots/vendor/mit/mit_primitive.hpp"

namespace ymd::robots::myactuator { 
using namespace ymd::literals;

enum class DeMsgError:uint8_t{

};

static constexpr uq32 degree001_to_turns(const uint16_t bits){
    // 目标：计算 (bits * 2^32) / 36000
    // 避免除法，使用乘法和移位
    // 公式：result ≈ (bits * M) >> S
    // 其中 M ≈ 2^S * 2^32 / 36000
    
    // 选择 S = 20 提供足够的精度（误差 < 0.5 LSB）
    // 计算 M = ceil(2^(32+S) / 36000)
    constexpr uint32_t S = 20;
    constexpr uint64_t M = ((1ULL << (32 + S)) + 36000 - 1) / 36000;
    
    // 计算 turns_bits = (bits * M) >> S
    const uint64_t product = static_cast<uint64_t>(bits) * M;
    const uint32_t turns_bits = static_cast<uint32_t>(product >> S);
    
    return uq32::from_bits(turns_bits);
}

static constexpr uq32 degree_to_turns(const uint16_t bits){
    constexpr uint32_t S = 20;
    constexpr uint64_t M = ((1ULL << (32 + S)) + 360 - 1) / 360;
    
    // 计算 turns_bits = (bits * M) >> S
    const uint64_t product = static_cast<uint64_t>(bits) * M;
    const uint32_t turns_bits = static_cast<uint32_t>(product >> S);
    
    return uq32::from_bits(turns_bits);
}

struct [[nodiscard]] SpeedCode_i16{
    using Self = SpeedCode_i16;

    int16_t bits;

    static constexpr Result<Self, DeMsgError> try_from_bits(const uint16_t bits){
        return Ok(Self{.bits = std::bit_cast<int16_t>(bits)});
    }
    constexpr iq8 to_dps() const {
        return bits;
    }
};

struct [[nodiscard]] SpeedLimitCode_u16{
    using Self = SpeedLimitCode_u16;    
    uint16_t bits;
    static constexpr Result<Self, DeMsgError> try_from_bits(const uint16_t bits){
        return Ok(Self{.bits = std::bit_cast<uint16_t>(bits)});
    }
    static constexpr SpeedLimitCode_u16 from_dps(const uq8 dps){
        return SpeedLimitCode_u16{static_cast<uint16_t>(dps)};
    }

    constexpr uq8 to_dps() const {
        return bits;
    }
};

struct [[nodiscard]] SpeedCtrlCode_i32{
    // 控制值 speedControl 为int32_t类型，对应实际转速为0.01dps/LSB,
    using Self = SpeedCtrlCode_i32;
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
    using Self = AccelCode_u32;
    uint32_t bits;

    static constexpr uq16 MAX_DPSS = 60000;
    static constexpr uq16 MIN_DPSS = 100;

    static constexpr uq16 MAX_TPSS = uq16(60000.0 / 360);
    static constexpr uq16 MIN_TPSS = uq16(100.0 / 360);

    static constexpr Result<Self, DeMsgError> try_from_bits(const uint32_t bits){
        return Ok(Self{.bits = std::bit_cast<uint32_t>(bits)});
    }
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
    using Self = DegreeCode_i16;
    int16_t bits;

    [[nodiscard]] constexpr Angular<iq16> to_angle() const {
        return Angular<iq16>::from_turns(degree_to_turns(bits));
    }
};


struct [[nodiscard]] LapAngleCode_u16{
    // 电机单圈角度circleAngle, 为 uint16_t类型数据， 以编码器零点为起始点，顺时针
    // 增加，再次到达零点时数值回0,单位0.01°LSB, 数值范围0~35999。
    uint16_t bits;
    constexpr Angular<uq32> to_angle() const {
        return Angular<uq32>::from_turns(degree001_to_turns(bits));
    }
};

struct [[nodiscard]] TemperatureCode_i8{
    uint8_t bits;

    [[nodiscard]] constexpr int8_t to_celsius() const {
        return std::bit_cast<int8_t>(bits);
    }
};

struct [[nodiscard]] VoltageCode_u16{
    uint16_t bits;

    [[nodiscard]] constexpr uq16 to_volts() const {
        return uq16(0.1) * bits;
    }
};

struct [[nodiscard]] CurrentCode_i16{
    int16_t bits;

    [[nodiscard]] constexpr iq16 to_amps() const {
        return iq16(0.01) * bits;
    }
};


struct [[nodiscard]] PositionCode_i32{
    // 0.01度每lsb
    // 至少需要iq15以表示全域数据
    int32_t bits;

    [[nodiscard]] constexpr Angular<iq15> to_angle() const {
        // 单次乘法，无除法
        const iiq47 product = static_cast<int64_t>(bits) * iiq47::from_rcp(36000);
        const int32_t iq15_turns_bits = static_cast<int32_t>(product.to_bits() >> 32);
        
        return Angular<iq15>::from_turns(iq15::from_bits(iq15_turns_bits));
    }
};


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

enum class [[nodiscard]] PidIndex:uint8_t{
    CurrentKp = 0x01,
    CurrentKi = 0x02,
    SpeedKp = 0x04,
    SpeedKi = 0x05,
    PositionKp = 0x07,
    PositionKi = 0x08,
    PositionKd = 0x09,
};

enum class [[nodiscard]] PlanAccelKind:uint8_t{
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
    GetMultiAngle = 0x60,
    GetMultiAngleWithoutOffset = 0x61,
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

enum class [[nodiscard]] Baudrate:uint8_t{
    RS485_115200 = 0,
    CAN_500K = 0,
    RS485_500K = 1,
    CAN_1M = 1
};

struct [[nodiscard]] MotorId{
    uint8_t count;
};

}
