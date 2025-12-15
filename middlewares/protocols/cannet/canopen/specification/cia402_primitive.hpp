#pragma once
#include <cstdint>

namespace ymd::canopen{

enum class CiA402State : uint8_t {
    NotReadyToSwitchOn = 0,
    SwitchOnDisabled = 1,
    ReadyToSwitchOn = 2,
    SwitchedOn = 3,
    OperationEnabled = 4,
    QuickStopActive = 5,
    FaultReactionActive = 6,
    Fault = 7
};


struct ControlWord {
    uint8_t switchOn : 1;
    uint8_t enableVoltage : 1;
    uint8_t quickStop : 1;
    uint8_t enableOperation : 1;
    uint8_t faultReset : 1;
    uint8_t halt : 1;
    uint8_t :2;
};

struct StatusWord {
    uint8_t readyToSwitchOn : 1;
    uint8_t switchedOn : 1;
    uint8_t operationEnabled : 1;
    uint8_t fault : 1;
    uint8_t voltageEnabled : 1;
    uint8_t quickStop : 1;
    uint8_t warning : 1;
    uint8_t targetReached : 1;
};

enum class MotionMode : uint8_t {
    NoMode = 0,
    ProfilePosition = 1,
    ProfileVelocity = 3,
    Torque = 4
};

}