#pragma once

#include <cstdint>
#include <span>



namespace ymd::robots::pdstepper{

static constexpr uint8_t FRAME_HEAD_TOEKN             = 0xC5;        /* 帧头 */
static constexpr uint8_t FRAME_TAIL_TOEKN             = 0x5C;        /* 帧尾 */

enum class [[nodiscard]] Command:uint8_t{
    /* 系统指令：(0x00~0x0F) */
    Idle                    = 0x00,      /* 空闲功能码 */
    CalibrateEncoder        = 0x01,      /* 校准编码器 */
    Reset                 = 0x02,      /* 复位重启 */
    RestoreFactory           = 0x03,      /* 恢复出厂设置 */
    SaveParam              = 0x04,      /* 参数保存 */

    /* 读参数指令：(0x20~0x3F) */
    GetSoftHardVer      = 0x20,      /* 读取软硬件版本信息 */
    GetFlux                = 0x21,      /* 读取电机磁链 */
    GetPhaseResInd      = 0x22,      /* 读取相电阻和相电感 */
    GetPhaseCurrent           = 0x23,      /* 读取相电流 */
    GetVoltage                = 0x24,      /* 读取总线电压 */
    GetCurrentPidParaments             = 0x25,      /* 读取电流环PID参数 */
    GetSpeedPidParaments          = 0x26,      /* 读取速度环PID参数 */
    GetPositionPidParaments            = 0x27,      /* 读取位置环PID参数 */
    GetTotalPulse        = 0x28,      /* 读取输入累计脉冲数 */
    GetRotateSpeed       = 0x29,      /* 读电机实时转速 */
    GetPos                = 0x2A,      /* 读取电机实时位置 */
    GetPosError          = 0x2B,      /* 读取电机位置误差 */
    GetMotorSta          = 0x2C,      /* 读取电机运行状态 */
    GetClogFlag          = 0x2D,      /* 读取堵转标志 */
    GetClogCur           = 0x2E,      /* 读取堵转电流 */
    GetEnableSta         = 0x2F,      /* 读使能状态 */
    GetArrivedSta        = 0x30,      /* 读取到位状态 */
    GetSysParam          = 0x31,      /* 读取系统参数 */
    GetDriveParams       = 0x32,      /* 读取驱动参数 */

    /* 设置参数指令：(0x60~0x7F) */
    SetSlaveAdd           = 0x60,      /* 设置从机地址 */
    SetGroupAdd           = 0x61,      /* 设置分组地址 */
    SetMode                = 0x62,      /* 设置工作模式 */
    SetPosPid             = 0x63,      /* 设置位置环PID */
    SetPosTorque          = 0x64,      /* 设置位置环最大力矩限制 */
    SetStep                = 0x65,      /* 设置细分 */
    SetMa                  = 0x66,      /* 设置目标电流 */
    SetUartBaud           = 0x67,      /* 设置串口波特率 */
    SetCanBaud            = 0x68,      /* 设置CAN波特率 */
    SetModbus              = 0x69,      /* 设置MODBUS协议 */
    SetClogPro            = 0x6A,      /* 设置堵转保护 */
    SetClogCur            = 0x6B,      /* 设置堵转电流 */
    SetCanId              = 0x6C,      /* 设置CAN_ID */
    SetDirLevel           = 0x6D,      /* 设置DIR正转电平 */
    SetEnLevel            = 0x6E,      /* 设置EN脚有效电平 */
    SetCmdEcho            = 0x6F,      /* 设置指令回响 */
    SetKeyLock            = 0x70,      /* 设置按键锁定 */
    SetAutoNotDisplay    = 0x71,      /* 设置自动熄屏 */
    SetIoStartLevel      = 0x72,      /* 设置IO启动电平 */
    SetSpeedPid           = 0x73,      /* 设置速度环PID */
    
    /* 限位回零相关指令：(0x90~0x9F) */
    OriginSetLeftPos     = 0x90,      /* 设左限位原点位置 */
    OriginLimitHome       = 0x91,      /* 有无限位回零 */
    OriginTrig             = 0x92,      /* 触发回零 */
    OriginBreak            = 0x93,      /* 强制中断并退出回零操作 */
    OriginReadParams      = 0x94,      /* 读取回零参数 */
    OriginSetParams       = 0x95,      /* 修改原点回零超时时间 */
    OriginReadSta         = 0x96,      /* 读取回零状态 */
    OriginAotoZero        = 0x97,      /* 上电自动回零设置 */
    OriginSetRightPos    = 0x98,      /* 设右限位原点位置 */
    OriginSwitch           = 0x99,      /* 左右限位开关 */
    
    /* 运动控制相关指令：(0xE0~0xFF) */
    OpenloopSpeedMode           = 0xE0,      /* 开环速度模式控制 */
    OpenloopPosMode             = 0xE1,      /* 开环绝对位置模式控制 */
    OpenloopPosRelMode         = 0xE2,      /* 开环相对位置模式控制 */
    OpenloopPulsesMode          = 0xE3,      /* 开环脉冲模式 */
    
    IoRunMode             = 0xE4,      /* IO启停模式 */
    
    TorqueMode             = 0xF0,      /* 力矩模式控制 */
    SpeedMode              = 0xF1,      /* 速度模式控制 */
    PosMode                = 0xF2,      /* 绝对位置模式控制 */
    PosRelMode            = 0xF3,      /* 相对位置模式控制 */
    PulsesMode             = 0xF4,      /* 脉冲模式 */
    PulseWidthPosMode    = 0xF5,      /* 脉宽位置模式 */
    PulseWidthMaMode     = 0xF6,      /* 脉宽电流模式 */
    PulseWidthSpeedMode  = 0xF7,      /* 脉宽速度模式 */
    AngleZero              = 0xF8,      /* 将当前的位置清零 */
    ClearClogPro          = 0xF9,      /* 解除堵转状态 */
    Enable            = 0xFA,      /* 电机使能控制 */
    ClearState             = 0xFB,      /* 清除状态（堵转、刹车，失能） */
    QuickStop                = 0xFC,      /* 立即停止（刹车） */
};

enum class [[nodiscard]] OperateErrc:uint8_t{
    Ok = 0x01,
    FrameTooShort = 0xe1,
    InvalidHeader = 0xe2,
    Inva
};

enum class [[nodiscard]] CalibrateState:uint8_t{
    Idle = 0x00,
    Progressing,
    Failed,
    Succeed
};


}