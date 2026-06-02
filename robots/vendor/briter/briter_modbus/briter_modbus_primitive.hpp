#pragma once

#include <cstdint>

namespace ymd::robots::briter{


enum class [[nodiscard]] FaultCode : uint8_t {
    None                = 0,    // 无错误
    OverVoltage         = 1,    // 过压
    UnderVoltage        = 2,    // 欠压
    OverCurrent         = 3,    // 绝对值超过最大电流
    MosOverTemperature  = 4,    // MOS管过温
    McuUnderVoltage     = 5,    // MCU欠压
    WatchdogReset       = 6,    // 看门狗触发后启动
    SpiDriverError      = 7,    // SPI接口驱动器错误
    FlashDamaged        = 8,    // FLASH损毁
    UPhaseCurrentOffset = 9,    // U相电流传感器偏移过大
    VPhaseCurrentOffset = 10,   // V相电流传感器偏移过大
    WPhaseCurrentOffset = 11,   // W相电流传感器偏移过大
    ThreePhaseUnbalance = 12,   // 三相电流不平衡
    MotorConfigDamaged  = 13,   // FLASH中电机配置损毁
    AppConfigDamaged    = 14,   // FLASH中应用配置损毁
    CanOpenHeartbeatErr = 15,   // CANOPEN心跳错误
    Stall               = 16,   // 堵转
    StallSpeed          = 17,   // 失速
    OverDeviation       = 18,   // 超差
    EncoderNoZSignal    = 19    // 编码器未找到Z信号
};

enum class [[nodiscard]] HommingState:uint8_t{
    Ok = 0,
    IoNotArmed = uint8_t(-1),
    Aborted = uint8_t(-2)
};

// 高 8 位：回零状态；0 代表回零中，1 代表回零完成。
// 低 8 位：回零代码；0 代表回零成功；-1 代表相应 IO 未设置功能；
// -2 代表中止回零
struct [[nodiscard]] HommingStatus{
    HommingState homming_state;
    bool is_homming_complete;
};


static_assert(sizeof(HommingStatus) == 2);

enum class ControlMode:uint16_t{
    // 0: 电流控制模式
    // 1：转速控制模式
    // 2：占空比控制模式
    // 3：绝对位置控制模式
    // 4：相对上一次目标位置控制模式
    // 5：相对当前位置控制模式
    // 6：刹车电流控制模式
    // 7：手刹电流控制模式
    // 8：回零模式
    // 9：回零停止
    // 10： 电流爬升模式
    // 0xFFFF：空模式

    None = 0xffff,
    Current = 0,
    Speed = 1,
    Duty = 2,
    AbsPosition = 3,
    RelLastTargetPosition = 4,
    RalNowPosition = 5,

};


// 高 8 位 NC；
// 低 8 位：0b00000000；从低位开始编号 in1；为 1 有效，0 无效；
struct [[nodiscard]] IoState final{
    uint16_t bits;
};

struct [[nodiscard]] ErrorMask final{
    uint16_t bits;
};


struct [[nodiscard]] CurrentCode final{
    //10ma
    uint16_t bits;
};

struct [[nodiscard]] ErpmCode final{
    //10ma
    int32_t bits;
};



enum class RegAddr : uint16_t {
    // ========================== 输入寄存器（只读 RO）==========================
    FaultInfo               = 5000,  // 故障信息

    // 此转速为电角度转速，erpm=rpm*磁极对数
    RealtimeSpeed           = 5001,  // 实时转速（低16位）
    RealtimeSpeedHigh       = 5002,  // 实时转速（高16位）
    RealtimeDuty            = 5003,  // 实时占空比
    RealtimePower           = 5004,  // 实时功率(W)
    RealtimeVoltage         = 5005,  // 实时输入电压(V)
    RealtimeMotorCurrent    = 5006,  // 实时电机电流(10mA)
    RealtimeBusCurrent      = 5007,  // 实时总线电流(10mA)
    RealtimeTemp            = 5008,  // 实时温度 (摄氏度)
    RealtimeAngle           = 5009,  // 实时角度（0.01度）
    RealtimePos             = 5010,  // 实时位置（低16位，0.01度）
    RealtimePosHigh         = 5011,  // 实时位置（高16位，0.01度）
    HomingStatus            = 5012,  // 回零状态
    FindZSignal             = 5013,  // 是否找到Z信号(为 1 代表找到 Z 信号；0 未找到 Z 信号)
    HistoryErr1             = 5014,  // 历史错误1
    HistoryErr2             = 5015,  // 历史错误2
    HistoryErr3             = 5016,  // 历史错误3
    HistoryErr4             = 5017,  // 历史错误4
    HistoryErr5             = 5018,  // 历史错误5
    InputIOState            = 5019,  // 输入IO状态
    OutputIOState           = 5020,  // 输出IO状态

    // ========================== 保持寄存器（读写 RW）==========================
    Heartbeat                       = 6000,  // 心跳包（需周期更新，否则停机）
    CtrlMode                        = 6001,  // 控制模式
    SetCurrent                 = 6002,  // 设定电流
    SetSpeedLow                = 6003,  // 设定转速（低16位）
    SetSpeedHigh               = 6004,  // 设定转速（高16位）
    SetDuty                    = 6005,  // 设定占空比
    SetAbsPosLow               = 6006,  // 设定绝对位置（低16位）
    SetAbsPosHigh              = 6007,  // 设定绝对位置（高16位）
    SetRelPosLastTargetLow     = 6008,  // 设定相对位置（相对上一目标，低16位）
    SetRelPosLastTargetHigh    = 6009,  // 设定相对位置（相对上一目标，高16位）
    SetRelPosCurrent           = 6010,  // 设定相对位置（相对当前，低16位）
    SetRelPosCurrentHigh       = 6011,  // 设定相对位置（相对当前，高16位）
    SetCurrentPosLow           = 6012,  // 设定当前位置（低16位）
    SetCurrentPosHigh          = 6013,  // 设定当前位置（高16位）
    SetBrakeCurrent            = 6014,  // 设定刹车电流
    SetHandbrakeCurrent        = 6015,  // 设定手刹电流
    SpeedLoopAccelLow               = 6016,  // 速度环加速度（低16位）
    SpeedLoopAccelHigh              = 6017,  // 速度环加速度（高16位）
    TrajMaxSpeedLow                 = 6018,  // 轨迹最大速度（低16位）
    TrajMaxSpeedHigh                = 6019,  // 轨迹最大速度（高16位）
    TrajMaxAccelLow                 = 6020,  // 轨迹最大加速度（低16位）
    TrajMaxAccelHigh                = 6021,  // 轨迹最大加速度（高16位）
    TrajMaxDecelLow                 = 6022,  // 轨迹最大减速度（低16位）
    TrajMaxDecelHigh                = 6023,  // 轨迹最大减速度（高16位）
    ActiveMotorConfig               = 6024,  // 当前生效电机配置表
    SpeedLoopDecelLow               = 6025,  // 速度环减速度（低16位）
    SpeedLoopDecelHigh              = 6026,  // 速度环减速度（高16位）
    HommingMode                     = 6027,  // 回零模式
    MaxCloseLoopCurrent             = 6028,  // 闭环模式最大电流
    CurrentRampAccel                = 6029,  // 电流爬升加速度
    SetCurrentRamp             = 6030   // 电流爬升模式设定电流
};



}