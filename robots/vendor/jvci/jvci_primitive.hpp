#pragma once

#include "core/math/float/fp32.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/math/real.hpp"
#include "primitive/can/bxcan_frame.hpp"

namespace ymd::robots::jvci{


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

//1-127
struct NodeId{
    uint8_t bits;

    constexpr uint8_t to_u8() const {
        return bits;
    }
};



static constexpr hal::CanStdId REQUEST_ID_BASE = hal::CanStdId::from_u11(0x600);
static constexpr hal::CanStdId RESPONSE_ID_BASE = hal::CanStdId::from_u11(0x580);

static constexpr hal::CanStdId make_request_canid(const NodeId node_id){
    return hal::CanStdId::from_u11(REQUEST_ID_BASE.to_u11() + node_id.to_u8());
}

static constexpr hal::CanStdId make_response_canid(const NodeId node_id){
    return hal::CanStdId::from_u11(RESPONSE_ID_BASE.to_u11() + node_id.to_u8());
}


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


// 故障指示器位字段（扩展至文档所有故障位，32bit拆分为高低16位）
struct [[nodiscard]] FaultIndicatorL { // 低16位故障标识 [0,16)
    uint16_t bits;

    template <typename Self>
    [[nodiscard]] constexpr auto calibrate_current_overload(this Self&& self) {
        return ymd::make_bitfield_proxy<0, 1, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto phase_resistor_large(this Self&& self) {
        return ymd::make_bitfield_proxy<1, 2, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto motor_phase_loss(this Self&& self) {
        return ymd::make_bitfield_proxy<2, 3, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto calibrate_current_fluctuate(this Self&& self) {
        return ymd::make_bitfield_proxy<3, 4, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto calibrate_inductance_large(this Self&& self) {
        return ymd::make_bitfield_proxy<4, 5, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_bandwidth_error(this Self&& self) {
        return ymd::make_bitfield_proxy<5, 6, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_spi_error(this Self&& self) {
        return ymd::make_bitfield_proxy<6, 7, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_type_error(this Self&& self) {
        return ymd::make_bitfield_proxy<7, 8, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto hall_motor_uncalibrated(this Self&& self) {
        return ymd::make_bitfield_proxy<8, 9, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_data_unread(this Self&& self) {
        return ymd::make_bitfield_proxy<9, 10, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto encoder_cpr_error(this Self&& self) {
        return ymd::make_bitfield_proxy<10, 11, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto need_calibrate_before_loop(this Self&& self) {
        return ymd::make_bitfield_proxy<11, 12, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto run_state_error(this Self&& self) {
        return ymd::make_bitfield_proxy<12, 13, ymd::BooleanOk>(self.bits);
    }

    template <typename Self>
    [[nodiscard]] constexpr auto hall_motor_signal_error(this Self&& self) {
        return ymd::make_bitfield_proxy<15, 16, ymd::BooleanOk>(self.bits);
    }
};

struct [[nodiscard]] FaultIndicatorH { // 高16位故障标识 [16,32)
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


struct [[nodiscard]] BusbarVoltageCode final {
    using Self = BusbarVoltageCode;

    uint16_t bits; 
    
    static constexpr float SCALE = 0.01;
    static constexpr float INV_SCALE = 1.0 / SCALE;
    static constexpr uint16_t OFFSET = 0;

    static constexpr BusbarVoltageCode from_volt(const float volt){
        const auto bits = bounded_encode_to<uint16_t>(volt, INV_SCALE, OFFSET);
        return BusbarVoltageCode{.bits = bits};
    }

    [[nodiscard]] constexpr float to_volt() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

struct [[nodiscard]] SpeedCode final {
    using Self = SpeedCode;

    int32_t bits; 
    
    static constexpr float SCALE = 0.01;
    static constexpr float INV_SCALE = 1.0 / SCALE;
    static constexpr int32_t OFFSET = 0;

    static constexpr SpeedCode from_rpm(const float rpm){
        const auto bits = bounded_encode_to<int32_t>(rpm, INV_SCALE, OFFSET);
        return SpeedCode{.bits = bits};
    }

    [[nodiscard]] constexpr float to_rpm() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

struct [[nodiscard]] PositionCode final {
    using Self = PositionCode;

    int32_t bits; 
    

    static constexpr PositionCode from_degrees(const float degrees){
        const auto bits = bounded_encode_to<int32_t>(degrees, INV_SCALE, OFFSET);
        return PositionCode{.bits = bits};
    }

    [[nodiscard]] constexpr float to_degrees() const {
        return decode_from(bits, SCALE, OFFSET);
    }
};

}