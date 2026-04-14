#pragma once

#include "core/math/float/fp32.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/math/real.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"
#include "primitive/can/bxcan_frame.hpp"


namespace ymd::robots::jvci{

namespace utils{
static constexpr iq16 degree001_to_turns(const int32_t bits){
    constexpr uint32_t S = 26;
    constexpr int64_t M = static_cast<int64_t>(((1ULL << (16 + S)) + 36000 - 1) / 36000);
    
    const int64_t product = static_cast<int64_t>(bits) * M;
    const int32_t turns_bits = static_cast<int32_t>(product >> S);
    
    return iq16::from_bits(turns_bits);
}

static constexpr int32_t turns_to_degree001(const iq16 turns){
    const int32_t turns_bits = turns.to_bits();
    const int32_t degree001 = static_cast<int32_t>((static_cast<int64_t>(turns_bits) * 36000LL) >> 16);
    return degree001;
}
}


struct [[nodiscard]] BooleanOk final{
    bool bit;

    [[nodiscard]] constexpr bool is_ok() const {return bit == 0;}
    [[nodiscard]] constexpr bool is_err() const {return bit == 1;}

    static constexpr BooleanOk ok(){
        return {false};
    }

    static constexpr BooleanOk err(){
        return {true};
    }
};

//1-127 驱动器节点ID，范围1-127（用于生成CAN标准帧ID）
struct [[nodiscard]] NodeId final{
    uint8_t bits;

    static constexpr Option<NodeId> try_from_u8(const uint8_t b){
        if(b == 0) return None;
        if(b > 127) return None;
        return Some(NodeId{.bits = static_cast<uint8_t>(b)});
    }



    constexpr uint8_t to_u8() const {
        return bits;
    }
};



/// CAN 标准ID 基址定义
static constexpr hal::CanStdId REQUEST_ID_BASE = hal::CanStdId::from_u11(0x600);   ///< 请求帧基础ID (0x600)
static constexpr hal::CanStdId RESPONSE_ID_BASE = hal::CanStdId::from_u11(0x580); ///< 响应帧基础ID (0x580)

/// 生成请求帧的 CAN ID
/// @param node_id 节点ID (范围1-127)，最终CAN ID = 0x600 + node_id
/// @return 计算后的请求帧 CAN ID
static constexpr hal::CanStdId make_request_canid(const NodeId node_id){
    return hal::CanStdId::from_u11(REQUEST_ID_BASE.to_u11() + node_id.to_u8());
}

/// 生成响应帧的 CAN ID
/// @param node_id 节点ID (范围1-127)，最终CAN ID = 0x580 + node_id
/// @return 计算后的响应帧 CAN ID
static constexpr hal::CanStdId make_response_canid(const NodeId node_id){
    return hal::CanStdId::from_u11(RESPONSE_ID_BASE.to_u11() + node_id.to_u8());
}

static constexpr Option<NodeId> exact_response_canid(const hal::CanStdId canid){
    const uint16_t id_u11 = canid.to_u11();
    int32_t offset = static_cast<int32_t>(id_u11) - static_cast<int32_t>(RESPONSE_ID_BASE.to_u11());
    if(offset <= 0) return None;
    if(offset > 127) return None;
    return NodeId::try_from_u8(static_cast<uint8_t>(offset));
}


/// 带界限的编码函数：将浮点值编码为整数，自动裁剪到目标整数类型的范围
/// 公式: bits = (raw * inv_scale) + offset，然后 clamp 到 [T_min, T_max]
template<typename T>
static constexpr T bounded_encode_to(const float raw, const float inv_scale, const T offset){
    using ExtendedIntegral = int32_t;
    const ExtendedIntegral bits = static_cast<ExtendedIntegral>(raw * inv_scale) + static_cast<ExtendedIntegral>(offset);
    return static_cast<T>(std::clamp(
        bits,
        static_cast<ExtendedIntegral>(std::numeric_limits<T>::min()),
        static_cast<ExtendedIntegral>(std::numeric_limits<T>::max())
    ));
}

/// 解码函数：将整数码字解码为浮点值
/// 公式: result = (bits - offset) * scale
template<typename T>
static constexpr float decode_from(const T bits, const float scale, const T offset){
    return (static_cast<float>(bits) - static_cast<float>(offset)) * scale;
}

// 命令字
enum class [[nodiscard]] Command : uint8_t {
    ReadReg16     = 0x4B, // 读取单个寄存器存储的 16 位数据，如电压、温度等
    ReadReg32     = 0x43, // 读取多字节寄存器数据（32位），如速度、位置、错误信息
    WriteReg16    = 0x2B, // 写入单寄存器数据（16位），用于控制模式切换、功能指令
    WriteReg32    = 0x23, // 写入多字节寄存器数据（32位），用于设置带正负值的参数
    CmdPV         = 0x24, // PV指令操作：位置-速度指令
    CmdPVT        = 0x25, // PVT指令操作：位置-速度-力矩指令
    Response      = 0x2A  // 驱动器回复指令：响应上位机的反馈标识
};

enum class [[nodiscard]] ControlMode:uint8_t{
    Torque = 0,
    Speed = 1,
    TrapezoidPosition = 2,
    FilteredPosition = 3,
    DirectPosition = 4,
    LowSpeed = 5
};

// 寄存器地址
enum class [[nodiscard]] RegAddr : uint16_t {
    HardwareVersion    = 0x0002, // 只读 默认0x3205
    FirwareVersion     = 0x0003, // 只读 默认0x2807
    PowerVoltage       = 0x0004, // 只读 ×10 电源电压 0-100V
    BusCurrent         = 0x0005, // 只读 ×100 母线电流 ±20A
    RealSpeedH         = 0x0006, // 只读 ×100 实时速度H ±10000rpm
    RealPositionH      = 0x0007, // 只读 ×100 实时位置H
    RealPositionL      = 0x0008, // 只读 ×100 实时位置L
    Reserved0009       = 0x0009, // 只读 预留
    DriverTemperature  = 0x000A, // 只读 ×10 驱动器温度 0-150°C
    MotorTemperature   = 0x000B, // 只读 ×10 电机温度 0-150°C
    ErrorInfoH         = 0x000C, // 只读 错误信息H 32bit
    ErrorInfoL         = 0x000D, // 只读 错误信息L 32bit
    SetTorque          = 0x0020, // 读写 ×100 设定力矩 ±100Nm
    SetSpeedH          = 0x0021, // 读写 ×100 设定速度H ±10000rpm
    SetSpeed           = 0x0022, // 读写 ×100 设定速度
    SetAbsPositionH    = 0x0023, // 读写 ×100 设定绝对位置H
    SetAbsPositionL    = 0x0024, // 读写 ×100 设定绝对位置L
    SetRelPositionH    = 0x0025, // 读写 ×100 设定相对位置H
    SetRelPositionL    = 0x0026, // 读写 ×100 设定相对位置L
    SetLowSpeed        = 0x0027, // 读写 ×100 设定低速 ±300rpm
    ControlMode        = 0x0060, // 读写 控制模式 0-56
    IdleState          = 0x00A0, // 只写 空闲状态 0=无效 1=执行
    CalibrateMotor     = 0x00A1, // 只写 校准电机 0=无效 1=执行
    EnterCloseLoop     = 0x00A2, // 只写 进入闭环 0=无效 1=执行
    EraseParam         = 0x00A3, // 只写 擦除参数 0=无效 1=执行
    SaveParam          = 0x00A4, // 只写 保存参数 0=无效 1=执行
    RestartDriver      = 0x00A5, // 只写 重启驱动器 0=无效 1=执行
    SetOrigin          = 0x00A6, // 只写 设置原点 0=无效 1=执行
    SetTempOrigin      = 0x00A7  // 只写 设置临时原点 0=无效 1=执行
};


/// 故障指示器低16位（bit[0,16)）
/// 对应32位错误信息的低16位，包含13个故障位（bit13和bit14未定义）
struct [[nodiscard]] FaultIndicatorL final{
    uint16_t bits;

    template <typename Self>
    [[nodiscard]] constexpr auto calibrate_current_overload(this Self&& self) {
        return ymd::make_bitfield_proxy<0, 1, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto phase_resistor_large(this Self&& self) {
        return ymd::make_bitfield_proxy<1, 2, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto motor_phase_loss(this Self&& self) {
        return ymd::make_bitfield_proxy<2, 3, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto calibrate_current_fluctuate(this Self&& self) {
        return ymd::make_bitfield_proxy<3, 4, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto calibrate_inductance_large(this Self&& self) {
        return ymd::make_bitfield_proxy<4, 5, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_bandwidth_error(this Self&& self) {
        return ymd::make_bitfield_proxy<5, 6, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_spi_error(this Self&& self) {
        return ymd::make_bitfield_proxy<6, 7, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_type_error(this Self&& self) {
        return ymd::make_bitfield_proxy<7, 8, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto hall_motor_uncalibrated(this Self&& self) {
        return ymd::make_bitfield_proxy<8, 9, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_data_unread(this Self&& self) {
        return ymd::make_bitfield_proxy<9, 10, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_cpr_error(this Self&& self) {
        return ymd::make_bitfield_proxy<10, 11, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto need_calibrate_before_loop(this Self&& self) {
        return ymd::make_bitfield_proxy<11, 12, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto run_state_error(this Self&& self) {
        return ymd::make_bitfield_proxy<12, 13, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto hall_motor_signal_error(this Self&& self) {
        return ymd::make_bitfield_proxy<15, 16, BooleanOk>(self.bits);
    }
};

/// 故障指示器高16位（bit[16,32)）
/// 对应32位错误信息的高16位，包含7个故障/报警位（bit16、bit18未定义）
struct [[nodiscard]] FaultIndicatorH final{
    uint16_t bits;

    template <typename Self>
    [[nodiscard]] constexpr auto second_encoder_error(this Self&& self) {
        return ymd::make_bitfield_proxy<1, 2, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto jc2804_driver_error(this Self&& self) {
        return ymd::make_bitfield_proxy<3, 4, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto mos_overtemp_alarm(this Self&& self) {
        return ymd::make_bitfield_proxy<4, 5, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto motor_overtemp_alarm(this Self&& self) {
        return ymd::make_bitfield_proxy<5, 6, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto under_voltage_alarm(this Self&& self) {
        return ymd::make_bitfield_proxy<6, 7, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto over_voltage_alarm(this Self&& self) {
        return ymd::make_bitfield_proxy<7, 8, BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto over_current_alarm(this Self&& self) {
        return ymd::make_bitfield_proxy<8, 9, BooleanOk>(self.bits);
    }

};


/// 电源电压编码
/// 寄存器地址：0x0004，倍数×10，范围：0-100V
/// 编码示例：12.3V → 0x007B (123)
struct [[nodiscard]] BusbarVoltageCode final {
    using Self = BusbarVoltageCode;

    uint16_t bits;

    static constexpr float SCALE = 0.1f;      ///< 反编码倍数：bits * 0.1 = volt
    static constexpr float INV_SCALE = 1.0 / SCALE; 
    static constexpr uint16_t OFFSET = 0;

    /// 从电压值（单位：V）编码为电源电压码
    /// @param volt 电压值，范围 0-100V
    /// @return 编码后的电源电压码
    static constexpr BusbarVoltageCode from_volt(const float volt){
        const auto bits = bounded_encode_to<uint16_t>(volt, INV_SCALE, OFFSET);
        return BusbarVoltageCode{.bits = bits};
    }

    /// 将电源电压码解码为电压值（单位：V）
    [[nodiscard]] constexpr float to_volt() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

/// 速度编码（实时速度或设定速度）
/// 寄存器地址：0x0006(实时速度H), 0x0021(设定速度H), 0x0022(设定速度)
/// 倍数×100，范围：±10000rpm
/// 编码示例：500rpm → 0x0000C350 (50000)，-500.23rpm → 0xFFFF3C99 (-50023)
struct [[nodiscard]] SpeedCode final {
    using Self = SpeedCode;

    int32_t bits;

    static constexpr float SCALE = 0.01f;     ///< 反编码倍数：bits * 0.01 = rpm
    static constexpr float INV_SCALE = 1.0 / SCALE;
    static constexpr int32_t OFFSET = 0;

    /// 从转速值（单位：rpm）编码为速度码
    /// @param rpm 转速值，范围 ±10000rpm
    /// @return 编码后的速度码
    static constexpr SpeedCode from_rpm(const float rpm){
        const auto bits = bounded_encode_to<int32_t>(rpm, INV_SCALE, OFFSET);
        return SpeedCode{.bits = bits};
    }

    /// 将速度码解码为转速值（单位：rpm）
    [[nodiscard]] constexpr float to_rpm() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

/// 位置编码（实时位置或设定位置）
/// 寄存器地址：0x0007/0x0008(实时位置H/L), 0x0023/0x0024(设定绝对位置H/L), 0x0025/0x0026(设定相对位置H/L)
/// 倍数×100，范围：-11796120°~11796480°
/// 编码示例：360° → 0x00008CA0 (36000)，-180.45° → 0xFFFFB983 (-18045)
struct [[nodiscard]] PositionCode final {
    using Self = PositionCode;

    int32_t bits;

    static constexpr float DEG_SCALE = 0.01f;     ///< 反编码倍数：bits * 0.01 = degrees
    static constexpr float INV_DEG_SCALE = 1.0 / DEG_SCALE;
    static constexpr int32_t OFFSET = 0;

    /// 从角度值（单位：°）编码为位置码
    /// @param degrees 角度值，范围 -11796120°~11796480°
    /// @return 编码后的位置码
    static constexpr PositionCode from_degrees(const float degrees){
        const auto bits = bounded_encode_to<int32_t>(degrees, INV_DEG_SCALE, OFFSET);
        return PositionCode{.bits = bits};
    }

    /// 将位置码解码为角度值（单位：°）
    [[nodiscard]] constexpr float to_degrees() const {
        return decode_from(bits, DEG_SCALE, OFFSET);
    }

    static constexpr PositionCode from_turns(const iq16 turns){
        const auto bits = utils::turns_to_degree001(turns);
        return PositionCode{.bits = bits};
    }

    [[nodiscard]] constexpr iq16 to_turns() const {
        return utils::degree001_to_turns(bits);
    }
};

/// 母线电流编码
/// 寄存器地址：0x0005
/// 倍数×100，范围：±20A
/// 编码示例：1A → 0x0064 (100)
struct [[nodiscard]] BusCurrentCode final {
    using Self = BusCurrentCode;

    int16_t bits;

    static constexpr float SCALE = 0.01f;     ///< 反编码倍数：bits * 0.01 = ampere
    static constexpr float INV_SCALE = 1.0 / SCALE;
    static constexpr int16_t OFFSET = 0;

    /// 从电流值（单位：A）编码为母线电流码
    /// @param ampere 电流值，范围 ±20A
    /// @return 编码后的母线电流码
    static constexpr BusCurrentCode from_ampere(const float ampere){
        const auto bits = bounded_encode_to<int16_t>(ampere, INV_SCALE, OFFSET);
        return BusCurrentCode{.bits = bits};
    }

    /// 将母线电流码解码为电流值（单位：A）
    [[nodiscard]] constexpr float to_ampere() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

/// 驱动器温度编码
/// 寄存器地址：0x000A
/// 倍数×10，范围：0-150°C
/// 编码示例：34.5°C → 0x0159 (345)
struct [[nodiscard]] DriverTemperatureCode final {
    using Self = DriverTemperatureCode;

    uint16_t bits;

    static constexpr float SCALE = 0.1f;      ///< 反编码倍数：bits * 0.1 = celsius
    static constexpr float INV_SCALE = 1.0 / SCALE; 
    static constexpr uint16_t OFFSET = 0;

    /// 从温度值（单位：°C）编码为驱动器温度码
    /// @param celsius 温度值，范围 0-150°C
    /// @return 编码后的驱动器温度码
    static constexpr DriverTemperatureCode from_celsius(const float celsius){
        const auto bits = bounded_encode_to<uint16_t>(celsius, INV_SCALE, OFFSET);
        return DriverTemperatureCode{.bits = bits};
    }

    /// 将驱动器温度码解码为温度值（单位：°C）
    [[nodiscard]] constexpr float to_celsius() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

/// 电机温度编码
/// 寄存器地址：0x000B
/// 倍数×10，范围：0-150°C
/// 编码示例：67.8°C → 0x0237 (567)
struct [[nodiscard]] MotorTemperatureCode final {
    using Self = MotorTemperatureCode;

    uint16_t bits;

    static constexpr float SCALE = 0.1f;      ///< 反编码倍数：bits * 0.1 = celsius
    static constexpr float INV_SCALE = 1.0 / SCALE; 
    static constexpr uint16_t OFFSET = 0;

    /// 从温度值（单位：°C）编码为电机温度码
    /// @param celsius 温度值，范围 0-150°C
    /// @return 编码后的电机温度码
    static constexpr MotorTemperatureCode from_celsius(const float celsius){
        const auto bits = bounded_encode_to<uint16_t>(celsius, INV_SCALE, OFFSET);
        return MotorTemperatureCode{.bits = bits};
    }

    /// 将电机温度码解码为温度值（单位：°C）
    [[nodiscard]] constexpr float to_celsius() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

/// 力矩编码
/// 寄存器地址：0x0020
/// 倍数×100，范围：±100Nm
/// 编码示例：0.2Nm → 0x0014 (20)
struct [[nodiscard]] TorqueCode final {
    using Self = TorqueCode;

    int16_t bits;

    static constexpr float SCALE = 0.01f;     ///< 反编码倍数：bits * 0.01 = newton_meter
    static constexpr float INV_SCALE = 1.0 / SCALE;
    static constexpr int16_t OFFSET = 0;

    /// 从力矩值（单位：Nm）编码为力矩码
    /// @param newton_meter 力矩值，范围 ±100Nm
    /// @return 编码后的力矩码
    static constexpr TorqueCode from_newton_meter(const float newton_meter){
        const auto bits = bounded_encode_to<int16_t>(newton_meter, INV_SCALE, OFFSET);
        return TorqueCode{.bits = bits};
    }

    /// 将力矩码解码为力矩值（单位：Nm）
    [[nodiscard]] constexpr float to_newton_meter() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};


struct [[nodiscard]] PvSpeedCode final {
    using Self = PvSpeedCode;

    int16_t bits;

    static constexpr float SCALE = 1.0f;     ///< 反编码倍数：bits * 0.01 = rpm
    static constexpr float INV_SCALE = 1.0 / SCALE;
    static constexpr int16_t OFFSET = 0;

    static constexpr PvSpeedCode from_rpm(const float rpm){
        const auto bits = bounded_encode_to<int16_t>(rpm, INV_SCALE, OFFSET);
        return PvSpeedCode{.bits = bits};
    }

    [[nodiscard]] constexpr float to_rpm() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

struct [[nodiscard]] PvTorqueCode final {
    using Self = PvTorqueCode;

    uint8_t bits;

    static constexpr float SCALE = 1.0f;     ///< 反编码倍数：bits * 0.01 = rpm
    static constexpr float INV_SCALE = 1.0 / SCALE;
    static constexpr uint8_t OFFSET = 0;

    static constexpr PvTorqueCode from_percents(const float rpm){
        const auto bits = bounded_encode_to<uint8_t>(rpm, INV_SCALE, OFFSET);
        return PvTorqueCode{.bits = bits};
    }

    [[nodiscard]] constexpr float to_percents() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};


/// 低速编码
/// 寄存器地址：0x0027
/// 倍数×100，范围：±300rpm
/// 编码示例：1rpm → 0x0064 (100)
struct [[nodiscard]] LowSpeedCode final {
    using Self = LowSpeedCode;

    int16_t bits;

    static constexpr float SCALE = 0.01f;     ///< 反编码倍数：bits * 0.01 = rpm
    static constexpr float INV_SCALE = 1.0 / SCALE;
    static constexpr int16_t OFFSET = 0;

    /// 从低速值（单位：rpm）编码为低速码
    /// @param rpm 低速值，范围 ±300rpm
    /// @return 编码后的低速码
    static constexpr LowSpeedCode from_rpm(const float rpm){
        const auto bits = bounded_encode_to<int16_t>(rpm, INV_SCALE, OFFSET);
        return LowSpeedCode{.bits = bits};
    }

    /// 将低速码解码为低速值（单位：rpm）
    [[nodiscard]] constexpr float to_rpm() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};


}