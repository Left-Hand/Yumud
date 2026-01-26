#pragma once

#include "j1939_primitive.hpp"
#include "core/utils/Errno.hpp"
namespace ymd::j1939{

struct [[nodiscard]] EngineTorqueMode final{
    enum class [[nodiscard]] Kind:uint8_t{
        NoRequest = 0b0000,
        AcceleratorPedal = 0b0001,
        CruiseControl = 0b0010,
        PTOGovernor = 0b0011,

        RoadSpeedGovernor = 0b0100,
        ASRControl = 0b0101,
        TransmissionControl = 0b0110,
        ABSControl = 0b0111,

        TorqueLimiting = 0b1000,
        HighSpeedGovernor = 0b1001,
        BrakingSystem = 0b1010,
        RemoteAccelerator = 0b1011,
    };
};



}