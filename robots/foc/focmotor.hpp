#pragma once

#include "stepper/archive/archive.hpp"
#include "stepper/ctrls/ctrls.hpp"
#include "protocol/can_protocol.hpp"
#include "protocol/ascii_protocol.hpp"

class AsciiProtocol;
class CanProtocol;

class FOCMotor{ 
public:
    using ErrorCode = StepperEnums::ErrorCode;
    using RunStatus = StepperEnums::RunStatus;
    using CtrlType = StepperEnums::CtrlType;
protected:
    using ExitFlag = StepperEnums::ExitFlag;
    using InitFlag = StepperEnums::InitFlag;

    using Range = Range_t<real_t>;

    using Switches = StepperUtils::Switches;
    using NodeId = StepperUtils::NodeId;

    struct Measurements{
        real_t accel = 0;
        real_t curr = 0;
        real_t spd = 0;
        real_t pos = 0;
    };

    Measurements measurements;
    CtrlLimits ctrl_limits;
    real_t target;

    friend class AsciiProtocol;
    friend class CanProtocol;

    std::optional<AsciiProtocol> ascii_protocol;
    std::optional<CanProtocol> can_protocol;
public:
    virtual bool loadArchive(const bool outen = false) = 0;
    virtual void saveArchive(const bool outen = false) = 0;
    virtual void removeArchive(const bool outen = false) = 0;

    virtual void freeze() = 0;
    virtual void setTargetCurrent(const real_t current) = 0;
    virtual void setTargetSpeed(const real_t speed) = 0;
    virtual void setTargetPosition(const real_t pos) = 0;
    virtual void setTargetTrapezoid(const real_t pos) = 0;
    virtual void setOpenLoopCurrent(const real_t current) = 0;
    virtual void setTargetVector(const real_t pos) = 0;
    virtual void setTargetTeach(const real_t max_curr) = 0;
    virtual void locateRelatively(const real_t pos = 0) = 0;

    virtual bool isActive() const = 0;
    virtual const volatile RunStatus & status() = 0;

    virtual real_t getSpeed() const = 0;
    virtual real_t getPosition() const = 0;
    virtual real_t getCurrent() const = 0;
    virtual real_t getAccel() const = 0;

    virtual void setCurrentLimit(const real_t max_current) = 0;
    virtual void setPositionLimit(const Range & clamp) = 0;
    virtual void setSpeedLimit(const real_t max_spd) = 0;
    virtual void setAccelLimit(const real_t max_acc) = 0;
    virtual void enable(const bool en = true) = 0;
    virtual void setNodeId(const NodeId _id) = 0;


    virtual void setNozzle(const real_t duty) = 0;
    virtual void triggerCali() = 0;
    virtual void reset() = 0;

    virtual uint8_t getNodeId() = 0;
};