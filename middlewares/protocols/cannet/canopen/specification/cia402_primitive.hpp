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
    uint8_t ready_to_switch_on : 1;
    uint8_t switched_on : 1;
    uint8_t operation_enabled : 1;
    uint8_t fault : 1;
    uint8_t voltage_enabled : 1;
    uint8_t quick_stop : 1;
    uint8_t warning : 1;
    uint8_t target_reached : 1;
};

enum class MotionMode : uint8_t {
    NoMode = 0,
    ProfilePosition = 1,
    ProfileVelocity = 3,
    Torque = 4
};

}