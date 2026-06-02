#pragma once

#include "../briter_primitive.hpp"

namespace ymd::robots::briter{



enum class ModbusRegAddr : uint16_t {
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
    RealtimePos          = 5010,  // 实时位置（低16位，0.01度）
    RealtimePosHigh         = 5011,  // 实时位置（高16位，0.01度）
    HommingStatus           = 5012,  // 回零状态
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
    SetCurrent                      = 6002,  // 设定电流
    SetSpeed                        = 6003,  // 设定转速（低16位）
    SetSpeedHigh                    = 6004,  // 设定转速（高16位）
    SetDuty                         = 6005,  // 设定占空比
    SetAbsPos                       = 6006,  // 设定绝对位置（低16位）
    SetAbsPosHigh                   = 6007,  // 设定绝对位置（高16位）
    SetRelPosLastTarget             = 6008,  // 设定相对位置（相对上一目标，低16位）
    SetRelPosLastTargetHigh         = 6009,  // 设定相对位置（相对上一目标，高16位）
    SetRelPosCurrent                = 6010,  // 设定相对位置（相对当前，低16位）
    SetRelPosCurrentHigh            = 6011,  // 设定相对位置（相对当前，高16位）
    SetCurrentPos                   = 6012,  // 设定当前位置（低16位）
    SetCurrentPosHigh               = 6013,  // 设定当前位置（高16位）
    SetBrakeCurrent                 = 6014,  // 设定刹车电流
    SetHandbrakeCurrent             = 6015,  // 设定手刹电流
    SpeedLoopAccel                  = 6016,  // 速度环加速度（低16位）
    SpeedLoopAccelHigh              = 6017,  // 速度环加速度（高16位）
    TrajMaxSpeed                    = 6018,  // 轨迹最大速度（低16位）
    TrajMaxSpeedHigh                = 6019,  // 轨迹最大速度（高16位）
    TrajMaxAccel                    = 6020,  // 轨迹最大加速度（低16位）
    TrajMaxAccelHigh                = 6021,  // 轨迹最大加速度（高16位）
    TrajMaxDecel                    = 6022,  // 轨迹最大减速度（低16位）
    TrajMaxDecelHigh                = 6023,  // 轨迹最大减速度（高16位）
    ActiveMotorConfig               = 6024,  // 当前生效电机配置表
    SpeedLoopDecel                  = 6025,  // 速度环减速度（低16位）
    SpeedLoopDecelHigh              = 6026,  // 速度环减速度（高16位）
    HommingMode                     = 6027,  // 回零模式
    MaxCloseLoopCurrent             = 6028,  // 闭环模式最大电流
    CurrentRampAccel                = 6029,  // 电流爬升加速度
    SetCurrentRamp                  = 6030   // 电流爬升模式设定电流
};



}