#pragma once

#include "core/math/float/fp32.hpp"
#include "core/math/real.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"
#include "primitive/can/bxcan_frame.hpp"


namespace ymd::robots::lkmotor{

enum class [[nodiscard]] Command:uint8_t{
    // 1.读取电机状态1和错误标志命令
    GetStatus1AndErrors = 0x9a,

    // 2.清除电机错误标志命令
    ClearErrors = 0x9b,

    // 3.读取电机状态2
    GetStatus2 = 0x9c,

    // 4.读取电机状态3
    GetStatus3 = 0x9d,

    // 5.电机关闭命令
    MotorOff = 0x80,

    // 6.电机运行命令
    MotorOn = 0x88,

    // 7.电机停止命令
    MotorStop = 0x81,

    // 8.抱闸器控制和状态读取命令 
    ManipulateBraker = 0x8c,

    // 9.开环控制命令
    OpenloopControl = 0xa0,

    // 10.转矩闭环控制命令
    TorqueClosedLoopControl = 0xa1,

    // 11.速度闭环控制命令
    SpeedClosedLoopControl = 0xa2,

    // 12.多圈位置闭环控制命令1
    MultiTurnPosClosedLoopControl1 = 0xa3,

    // 13.多圈位置闭环控制命令2
    MultiTurnPosClosedLoopControl2 = 0xa4,

    // 14.单圈位置闭环控制命令1
    SoloTurnPosClosedLoopControl1 = 0xa5,

    // 15.单圈位置闭环控制命令2 
    SoloLapPositionControl2 = 0xa6,

    // 16.增量位置闭环控制命令1
    IncrementalPosClosedLoopControl1 = 0xa7,

    // 17.增量位置闭环控制命令2
    IncrementalPosClosedLoopControl2 = 0xa8,

    // 18.读取控制参数命令
    ReadParam = 0xc0,

    // 19.写入控制参数命令
    WriteParam = 0xc1,

    // 20.读取电机编码器数据命令
    ReadEncoderData = 0x90,

    // 21.校准编码器命令
    CalibrateEncoder = 0x18,

    // 22.设置当前位置为电机零点（写入ROM）
    SetCurrentAsZeroPointRom = 0x19,

    // 23.读取多圈角度命令
    ReadMultiTurnAngle = 0x92,

    // 24.读取单圈角度命令
    ReadSingleTurnAngle = 0x94,

    // 25.设置当前位置为零点（写入RAM）
    SetCurrentAsZeroPointRam = 0x95,

    // 26.读取设定参数命令
    ReadSettingParam = 0x40,

    // 27.写入设定参数命令
    WriteSettingParam = 0x42,

    // 28.保存设定参数命令
    SaveSettingParam = 0x44,

    // 29.电机重启命令
    Reboot = 0x07
};


enum class [[nodiscard]] ParamId:uint8_t{
    SetPositionLoopPid = 0x0a,
    SetSpeedLoopPid = 0x0b,
    SetCurrentLoopPid = 0x0c,
    SetTorqueLimit = 0x1e,
    SetSpeedLimit = 0x20,
    SetAngleLimit = 0x22,
    SetCurrentRamp = 0x24,
    SetSpeedRamp = 0x26
};


enum class [[nodiscard]] SettingParamId:uint16_t{
    // 电机ID
    MotorId = 0x050a,
    // 总线类型
    BusType = 0x050b,
    // RS485波特率
    Rs485BaudRate = 0x050c,
    // CAN波特率
    CanBaudRate = 0x050d,
    // 最大功率
    MaxPower = 0x05e0,
    // 最大速度
    MaxSpeed = 0x05e2,
    // 最大角度
    MaxAngle = 0x05e4,
    // 电流斜率
    CurrentRamp = 0x05ea,
    // 速度斜率
    SpeedRamp = 0x05ec,
    // 位置环PID
    PositionLoopPid = 0x00a0,
    // 速度环PID
    SpeedLoopPid = 0x00a4,
    // 电流环PID
    CurrentLoopPid = 0x00a8
};


enum class [[nodiscard]] BrakerCommand:uint8_t{
    Establish = 0x00,
    Release = 0x01,
    GetStatus = 0x10
};


struct [[nodiscard]] NodeId final{
    //1~32
    uint8_t count;
    
    static constexpr uint16_t NUM_CANID_BASE = 0x140;
    static constexpr uint16_t NUM_MAX_MOTOR_COUNT = 32;

    constexpr hal::CanStdId to_canid() const {
        return hal::CanStdId::from_u11(NUM_CANID_BASE + count);
    }

    static constexpr Option<NodeId> try_from_canid(const hal::CanStdId canid){
        const int32_t id_offset = static_cast<int32_t>(canid.to_u11()) - int32_t(NUM_CANID_BASE);

        if(id_offset < 1) return None;
        if(id_offset > int32_t(NUM_MAX_MOTOR_COUNT)) return None;

        return Some(NodeId{.count = static_cast<uint8_t>(id_offset)});
    }
};


struct [[nodiscard]] ErrorState final{
    uint8_t under_voltage:1;      // 位0：低电压状态
    uint8_t over_voltage:1;       // 位1：高电压状态
    uint8_t driver_over_temp:1;   // 位2：驱动温度状态
    uint8_t motor_over_temp:1;    // 位3：电机温度状态
    uint8_t motor_over_current:1; // 位4：电机电流状态
    uint8_t motor_short_circuit:1;// 位5：电机短路状态
    uint8_t stall:1;              // 位6：堵转状态
    uint8_t input_signal_lost:1;  // 位7：输入信号状态

    constexpr uint8_t to_u8() const {
        return std::bit_cast<uint8_t>(*this);
    }

    static constexpr ErrorState from_u8(const uint8_t b){
        return std::bit_cast<ErrorState>(b);
    }
};


struct [[nodiscard]] MotorState final{
    uint8_t is_enabled:1;         // 0x00：电机开启；0x10：电机关闭
    uint8_t reserved:7;           // 保留位
};


struct [[nodiscard]] TemperatureCode final{
    int8_t bits;                  // 电机温度，单位1℃/LSB
};


struct [[nodiscard]] VoltageCode final{
    int16_t bits;          // 母线电压，0.01V/LSB
};


struct [[nodiscard]] Current final{
    int16_t bits;          // 母线电流，0.01A/LSB
};


struct [[nodiscard]] PowerCode final{
    int16_t bits;
};


struct [[nodiscard]] TorqueCode final{
    int16_t bits;
};


struct [[nodiscard]] SpeedCode final{
    int32_t bits;
};


struct [[nodiscard]] SpeedLimCode final{
    uint16_t bits;
};


struct [[nodiscard]] AngleCode final{
    uint32_t bits;
};


struct [[nodiscard]] AngleDeltaCode final{
    int32_t bits;
};


}