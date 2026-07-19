#pragma once

#include <cstdint>
#include "core/utils/bits/bitfield_proxy.hpp"
#include "primitive/can/can_id.hpp"

// 参考资料：
// https://github.com/codenocold/dgm/blob/main/Firmware/Firmware_app/Source/can.c


namespace ymd::robots::dgm{

//节点ID
struct [[nodiscard]] NodeId final{
    uint8_t bits;
};

// ============================================================================
// CAN 命令 ID
// ============================================================================
enum class [[nodiscard]] CommandId : uint8_t {
    // 控制类命令
    SetControlMode        = 0x00,  // 设置运行模式
    MotorEnable           = 0x01,  // 使能电机
    MotorDisable          = 0x02,  // 禁用电机

    // 目标值设置
    SetTorque             = 0x03,  // 设置扭矩命令
    SetVelocity           = 0x04,  // 设置速度命令
    SetPosition           = 0x05,  // 设置位置命令
    Sync                  = 0x06,  // 同步目标值

    // 标定类命令
    CalibrateStart            = 0x07,  // 启动标定
    CalibrateReport           = 0x08,  // 标定报告
    CalibrateAbort            = 0x09,  // 中止标定

    // 反齿槽标定
    AnticoggingStart      = 0x0A,  // 启动反齿槽标定
    AnticoggingReport     = 0x0B,  // 反齿槽报告
    AnticoggingAbort      = 0x0C,  // 中止反齿槽

    // 状态查询
    SetOrigin               = 0x0D,  // 设置零点
    ClearError            = 0x0E,  // 错误复位
    StatuswordGet         = 0x0F,  // 查询状态字
    StatuswordReport      = 0x10,  // 状态字上报

    // 数据查询
    GetValue1             = 0x11,  // 查询值1
    GetValue2             = 0x12,  // 查询值2

    // 系统命令
    Heartbeat             = 0x17,  // 心跳包
    SetConfig             = 0x18,  // 设置配置
    GetConfig             = 0x19,  // 读取配置
    SaveConfig         = 0x1A,  // 保存配置
    ResetConfig        = 0x1B,  // 复位配置
    GetFirmwareVersion          = 0x1C,  // 获取固件版本

    // DFU命令
    DfuStart              = 0x1D,  // 启动DFU
    DfuData               = 0x1E,  // DFU数据传输
    DfuEnd                = 0x1F,  // 完成DFU
};


// ============================================================================
// 电机控制模式
// ============================================================================
enum class [[nodiscard]] ControlMode : uint8_t {
    CurrentRamp       = 0,  // 电流斜坡模式
    VelocityRamp      = 1,  // 速度斜坡模式
    PositionFilter    = 2,  // 位置滤波器模式
    PositionProfile   = 3,  // 位置轮廓模式
};

// ============================================================================
// 电机控制状态
// ============================================================================
enum class [[nodiscard]] MotorState : uint8_t {
    BootUp      = 0,  // 启动阶段
    Idle        = 1,  // 空闲
    Run         = 2,  // 运行
    Calibration = 3,  // 标定中
    Anticogging = 4,  // 反齿槽标定中
};

// ============================================================================
// 电机状态字 - 错误位
// ============================================================================
struct [[nodiscard]] FaultFlags final{
    uint8_t bits;

    constexpr auto over_voltage(this auto && self) {
        return make_bitfield_proxy<0, 1, bool>(&self.bits);
    }

    constexpr auto under_voltage(this auto && self) {
        return make_bitfield_proxy<1, 2, bool>(&self.bits);
    }

    constexpr auto over_current(this auto && self) {
        return make_bitfield_proxy<2, 3, bool>(&self.bits);
    }

    constexpr auto driver_over_temperature(this auto && self) {
        return make_bitfield_proxy<3, 4, bool>(&self.bits);
    }

    constexpr auto ntc_over_temperature(this auto && self) {
        return make_bitfield_proxy<4, 5, bool>(&self.bits);
    }

    constexpr auto selftest_failed(this auto && self) {
        return make_bitfield_proxy<7, 8, bool>(&self.bits);
    }

};

// ============================================================================
// 标定阶段报告ID
// ============================================================================
enum class [[nodiscard]] CalibrationStep : int32_t {
    MotorResistanceStart  = 1,  // 电阻标定开始
    MotorResistanceEnd    = 2,  // 电阻标定结束
    MotorInductanceStart  = 3,  // 电感标定开始
    MotorInductanceEnd    = 4,  // 电感标定结束
    MotorPolePairs        = 5,  // 电机极对数
    EncoderDirection      = 6,  // 编码器方向
    EncoderOffset         = 7,  // 编码器偏移
    OffsetLutStart        = 10, // 偏移查找表开始
    OffsetLutEnd          = 10 + 128 - 1, // 偏移查找表结束
};

// ============================================================================
// 反齿槽标定报告ID
// ============================================================================
enum class [[nodiscard]] AnticoggingStep : int32_t {
    Complete = 5000,  // 反齿槽标定完成
};

// ============================================================================
// GET_VALUE 查询索引
// ============================================================================
enum class [[nodiscard]] ValueIndex : uint8_t {
    QuadCurrentFilt  = 0,  // Q轴滤波电流 (float)
    EncoderVelocity  = 1,  // 编码器速度 (float)
    EncoderPosition  = 2,  // 编码器位置 (float)
    BusVoltageFilt   = 3,  // 总线电压滤波值 (float)
    BusCurrentFilt   = 4,  // 总线电流滤波值 (float)
    PowerFilt        = 5,  // 功率滤波值 (float)
    DrvTemperature   = 6,  // DRV芯片温度 (float)
    NtcTemperature   = 7,  // NTC传感器温度 (float)
};

// ============================================================================
// 命令响应状态
// ============================================================================
enum class [[nodiscard]] ResponseStatus : uint8_t {
    Success = 0x00,  // 成功
    Failed  = 0xEE,  // 失败
};


struct [[nodiscard]] CobId final{
    uint16_t bits;


    constexpr auto command_id(this auto && self) {
        return make_bitfield_proxy<0, 5, CommandId>(&self.bits);
    }

    constexpr auto node_id(this auto && self) {
        return make_bitfield_proxy<5, 10, NodeId>(&self.bits);
    }

    constexpr auto echo(this auto && self) {
        return make_bitfield_proxy<10, 11, bool>(&self.bits);
    }

    static constexpr CobId from_canid(const hal::CanStdId canid){
        return CobId{.bits = canid.to_u11()};
    }

    constexpr hal::CanStdId to_canid() const {
        return hal::CanStdId::from_u11(bits);
    }
};


namespace req_msgs{

struct SetControlMode{
    static constexpr auto COMMAND_ID = CommandId::SetControlMode;

    ControlMode mode;


};

};

}