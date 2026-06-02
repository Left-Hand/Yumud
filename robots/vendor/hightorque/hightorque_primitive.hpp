#pragma once

#include <compare>
#include <cstdint>
#include "core/math/fixed/fixed.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"


// 高擎电机文档资料库：内含电机的各种资料文件，并且实时更新
// https://lingdongfangcheng.feishu.cn/wiki/space/7539498339620913154?ccm_open_type=lark_wiki_spaceLink&open_tab_from=wiki_home

// 1. CAN 波特率：
//   - 仲裁段：1 Mbps
//   - 数据段： 1Mbps
// 2. ID：由 16 位构成，其中 0x7F 是广播地址。
//   - 高 8 位：表示源地址：
//     - 最高位为 1 ：需要回复，相当于一个总开关，开启但不发送查询指令会返回一个数据段长度为 0 的帧。
//     - 最高位位 0：无需回复。
//     - 其余 7 位：信号源地址。
//   - 低 8 位：表示目的地址：
//     - 最高为 0。
//     - 其余 7 位表示目的地址。
//     例如：
//     - ID：0x8001
//       - 信号源地址为 0。
//       - 目的地址为 1。
//       - 最高位为 1，表示需要回复，即打开回复总开关。
//     - ID：0x100
//       - 信号源地址为 1。
//       - 目的地址为 0。
//       - 最高位为 0，表示无需回复，即关闭回复总开关。
// 3. 扩展帧和标准帧
//   - 发送
//     - 电机控制都可使用
//     - 读取电机状态需要使用扩展帧
//   - 电机反馈
//     - id在1-7的电机反馈状态使用标准帧
//     - id在8及以上的电机反馈状态使用扩展帧


namespace ymd::robots::hightorque{



enum class Mode:uint8_t{
    // 0 停止，清除错误
    Stop = 0,

    // 1 错误
    Error = 1,

    // 2，3，4 准备运行
    Ready = 2,

    // 5 PWM 模式
    Pwm = 5,

    // 6 电压模式
    Voltage = 6,

    // 7 foc电压模式
    FocVoltage = 7,

    // 8 DQ电压模式
    DqVoltage = 8,

    // 9 DQ电流模式
    DqCurrent = 9,

    // 10 位置模式
    Position = 10,

    // 11 超时模式
    Timeout = 11,

    // 12 零速模式
    ZeroSpeed = 12,

    // 13 范围模式
    Range = 13,

    // 14 测量电感模式
    MeasureInductance = 14,
    // 15 刹车模式
    Brake = 15
};

enum class Fault:uint8_t{
    // 1 DMA 数据流传输错误
    // 2 DMA 数据流 FIFO 错误
    // 3 UART 溢出错误
    // 4 UART 帧错误
    // 5 UART 噪声错误
    // 6 UART 缓冲区溢出错误
    // 7 UART 奇偶校验错误
    // … … 保留
    // 32 校准故障 校准过程中，编码器无法感知到磁铁
    // 33 电机驱动故障 多为欠压，电流不足
    // 34 过压 母线电压过大
    // 35 编码器故障 编码器读数错误
    // 36 电机未校准 电机还未进行校准（电机出厂都会校准一次）
    // 37 PWM周期过限 一般是内部固件错误
    // 38 温度过高 已超过最大配置温度
    // 39 起始位置超出限制 在位置界限之外尝试启动位置控制（出厂默认无位置限制）
    // 40 电压过低 电压太低
    // 41 配置已更改 在操作期间更改了需要停止的配置值
    // 42 角度无效 没有可用的有效换相编码器
    // 43 位置无效 没有可用的有效输出编码器
    // 44 驱动器使能故障 驱动芯片异常

    DmaTransferError = 1,
    DmaStreamFifoError = 2,
    UartOverflowError = 3,
    UartFrameError = 4,
    UartNoiseError = 5,
    UartBufferOverflowError = 6,
    UartParityError = 7,
    CalibrationError = 32,
    MotorDriverError = 33,
    OverVoltage = 34,
    EncoderError = 35,
    MotorNotCalibrated = 36,
    PwmPeriodTooLong = 37,
    TemperatureTooHigh = 38,
    StartPositionOutOfBounds = 39,
    UnderVoltage = 40,
    ConfigurationChanged = 41,
    AngleInvalid = 42,
    PositionInvalid = 43,
    DriverEnableError = 44,
};


struct [[nodiscard]] FaultCode final{
    using Self = FaultCode;
    static constexpr uint8_t MAX_NUM = static_cast<uint8_t>(Fault::DriverEnableError);
    uint8_t bits;

    static constexpr Option<Self> try_from_bits(const uint8_t b){
        if(b > MAX_NUM) return None;
        return Some(Self{b});
    }

    [[nodiscard]] bool is_ok() const noexcept {return bits == 0;}
    [[nodiscard]] bool is_err() const noexcept {return (bits > 0) and (bits <= MAX_NUM);}

    [[nodiscard]] Fault unwrap_err() const noexcept {
        if(not is_err()) __builtin_abort();
        return static_cast<Fault>(bits);
    }
    
    [[nodiscard]] Result<void, Fault> decode(){
        if(is_ok()) return Ok();
        return Err(static_cast<Fault>(bits));
    }
};


enum class [[nodiscard]] RegAddr:uint8_t{
    // 0x000 模式 R/W 电机运行模式（具体模式见 表2电机运行模式）
    Mode = 0x00,
    // 0x001 位置 R 电机输出轴位置
    Position = 0x01,
    // 0x002 速度 R 电机输出轴速度
    Speed = 0x02,
    // 0x003 转矩 R 电机的输出轴转矩
    Torque = 0x03,
    // 0x004 Q 相电流 R Q 相电流
    QCurrent = 0x04,
    // 0x005 D 相电流 R D 相电流
    DCurrent = 0x05,
    // 0x006 保留 保留
    Reserved1 = 0x06,
    // 0x00d 电压 R 输入电压
    Voltage = 0x0D,
    // 0x00e 温度 R 温度
    Temperature = 0x0E,
    // 0x00f 错误代码 R 具体错误代码见表3报错代码说明
    FaultCode = 0x0F,
    // 0x010 PWM相位A R/W PWM 模式下，控制 A 相的原始 PWM 值
    PwmPhaseA = 0x10,
    // 0x011 PWM相位B R/W PWM 模式下，控制 B 相的原始 PWM 值
    PwmPhaseB = 0x11,
    // 0x012 PWM相位C R/W PWM 模式下，控制 C 相的原始 PWM 值
    PwmPhaseC = 0x12,
    // 0x014 电压相位A R/W 电压模式下，控制施加到 A 相的电压
    VoltagePhaseA = 0x14,
    // 0x015 电压相位B R/W 电压模式下，控制施加到 B 相的电压
    VoltagePhaseB = 0x15,
    // 0x016 电压相位C R/W 电压模式下，控制施加到 C 相的电压
    VoltagePhaseC = 0x16,
    // 0x018 电压 FOC 角度 R/W 电压模式下，控制所需的电角度（没有乘以极对数）
    FocVoltageAngle = 0x18,
    // 0x019 电压 FOC 电压 R/W 电压聚焦模式下，控制所需的施加相电压
    FocVoltage = 0x19,
    // 0x01a D 电压 R/W DQ 电压模式下，控制 D 相电压
    DVoltage = 0x1A,
    // 高擎电机寄存器功能说明表
    // 0x01b Q 电压 R/W DQ 电压模式下，控制 Q 相电压
    QVoltage = 0x1B,
    // 0x01c Q 电流 R/W DQ 电流模式下，控制 Q 相电流
    QCurrentControl = 0x1C,
    // 0x01d D 电流 R/W DQ 电流模式下，控制 D 相电流
    DCurrentControl = 0x1D,
    // 0x020 位置指令 R/W 位置模式下，控制位置
    PositionCommand = 0x20,
    // 0x021 速度命令 R/W 位置模式下，控制速度
    SpeedCommand = 0x21,
    // 0x022 前馈扭矩 R/W 位置模式下，控制前馈转矩
    FeedforwardTorque = 0x22,
    // 0x023 Kp 比例 R/W 位置模式下，将比例控制项缩小给定因子
    KpRatio = 0x23,
    // 0x024 Kd 比例 R/W 位置模式下，将微分控制项缩小给定因子
    KdRatio = 0x24,
    // 0x025 最大扭矩 R/W 位置模式下，控制的最大扭矩
    MaxTorque = 0x25,
    // 0x026 停止位置 R/W 位置模式下，并且命令非零速度时，到达给定位置时停止运动 
    StopPosition = 0x26,
    // 0x027 保留 保留
    Reserved2 = 0x27,
    // 0x028 速度限制 R/W 全局速度限制
    SpeedLimit = 0x28,
    // 0x029 加速度限制 R/W 全局加速度限制
    AccelerationLimit = 0x29,
    // 0x02b Kp R/W 电机内部实际PID之中的Kp项
    Kp = 0x2B,
    // 0x02c Kd R/W 电机内部实际PID之中的Kd项
    Kd = 0x2C,
    // 0x02d Ki R/W 电机内部实际PID之中的Ki项
    Ki = 0x2D,
    // 0x030 比例扭矩 R PID 控制器中比例项的转矩
    ProportionalTorque = 0x30,
    // 0x031 积分扭矩 R PID 控制器中积分项的转矩
    IntegralTorque = 0x31,
    // 0x032 微分扭矩 R PID 控制器中微分项的转矩
    DifferentialTorque = 0x32,
    // 0x033 前馈扭矩 R PID 控制器中的前馈
    FeedforwardTorqueRead = 0x33,
    // 0x034 总控制扭矩 R 位置模式下，总控制扭矩
    TotalControlTorque = 0x34,
    // 0x040 下限 R/W 范围模式下，它控制最小允许位置
    LowerLimit = 0x40,
    // 0x041 上限 R/W 范围模式下，它控制最大允许位置
    UpperLimit = 0x41,
    // 0x042 前馈扭矩 0x022 寄存器的映射
    FeedforwardTorqueMap = 0x42,
    // 0x043 Kp 比例 0x023 寄存器的映射
    KpRatioMap = 0x43,
    // 0x044 Kd 比例 0x024 寄存器的映射
    KdRatioMap = 0x44,
    // 0x045 最大扭矩 0x025 寄存器的映射
    MaxTorqueMap = 0x45
};

static_assert(sizeof(RegAddr) == 1);

enum class [[nodiscard]] ElementType:uint8_t{
    B1 = 0b00,
    B2 = 0b01,
    B4 = 0b10,
    Float = 0b11
};


enum class [[nodiscard]] SlotCommand:uint8_t{
    Write = 0x00,
    Read = 0x01,
    Response = 0x02
};

struct [[nodiscard]] SlotSpecifier final{
    uint8_t quantity:2;
    ElementType element_type:2; 
    SlotCommand command:4;

    static constexpr SlotSpecifier from_bits(const uint8_t b){
        return std::bit_cast<SlotSpecifier>(b);
    }

    constexpr uint8_t to_bits() const noexcept {
        return std::bit_cast<uint8_t>(*this);
    }
};

static_assert(sizeof(SlotSpecifier) == 1);

}