#pragma once

#include "../briter_primitive.hpp"

namespace ymd::robots::briter{

enum class [[nodiscard]] CanCommand : uint8_t {
    Heartbeat = 0x00,
    SetCurrent = 0x01,
    SetSpeed = 0x02,
    SetDuty = 0x03,
    SetAbsPosition = 0x04,
    SetRelPositionLastTarget = 0x05,
    SetRelPosition = 0x06,
    SetThisPosition = 0x07,
    SetBrakeCurrent = 0x08,
    SetHandbrakeCurrent = 0x09,
    SetMaxTrajSpeed = 0x0b,
    SetMaxTrajAccel = 0x0c,
    SetMaxTrajDecel = 0x0d,
    SetActiveConfigIndex = 0x0e,
    SetHommingMode = 0x11,
    StopHomming = 0x12,
    HommingStatus = 0x13,
    SetMaxCurrent = 0x14,
    SetCurrentRampAccel = 0x15,
};


enum class [[nodiscard]] CanEnqueryId:uint8_t{
    FaultInfo = 0x00,
    RealtimeSpeed = 0x01,
    RealtimeDuty = 0x02,
    RealtimePower = 0x03,
    RealtimeVoltage = 0x04,
    RealtimeMotorCurrent = 0x05,
    RealtimeBusbarCurrent = 0x06,
    RealtimeTemperature = 0x07,
    RealtimeAngle = 0x08,
    RealtimePosition = 0x09,
    FindZSignal = 0x0a,
    HistoryErr1 = 0x0b,
    IOState = 0x0c
};


}