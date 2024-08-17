#pragma once

#include "types/float/bf16.hpp"
#include "hal/bus/can/can.hpp"

#include "robots/foc/stepper/constants.hpp"
#include <tuple>

namespace CANProtocolUtils{
    using E = bf16;
    using E_2 = std::tuple<E, E>;
    using E_3 = std::tuple<E, E, E>;
    using E_4 = std::tuple<E, E, E, E>;
}


class FOCMotor;

class CanProtocol{
protected:
    using Command = StepperEnums::Command;

    FOCMotor & motor;
    Can & can;

public:
    CanProtocol(FOCMotor & _motor, Can & _can):
        motor(_motor),
        can(_can){;}


    void readCan();
    virtual void parseCommand(const Command command, const CanMsg & msg);
};


