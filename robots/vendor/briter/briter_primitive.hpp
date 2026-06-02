
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
    int16_t bits;
};

struct [[nodiscard]] PositionCode final{
    int32_t bits;
};

struct [[nodiscard]] DutyCode final{
    int16_t bits;
};

struct [[nodiscard]] ErpmCode final{
    //10ma
    int32_t bits;
};




}