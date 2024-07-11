#pragma once

#include "cli.hpp"
#include "ctrls/ctrls.hpp"
#include "observer/observer.hpp"
#include "archive/archive.hpp"
#include "hal/adc/adcs/adc1.hpp"

class StepperConcept{ 
public:
    using ErrorCode = StepperEnums::ErrorCode;
    using RunStatus = StepperEnums::RunStatus;
    using CtrlType = StepperEnums::CtrlType;
protected:
    using ExitFlag = StepperEnums::ExitFlag;
    using InitFlag = StepperEnums::InitFlag;

    using Range = Range_t<real_t>;

    using Switches = StepperUtils::Switches;

    real_t est_speed;
    real_t est_pos;
    real_t run_current;
    real_t target;

public:
    virtual bool loadArchive(const bool outen = false) = 0;
    virtual void saveArchive(const bool outen = false) = 0;
    virtual void removeArchive(const bool outen = false) = 0;

    virtual void setTargetCurrent(const real_t current) = 0;
    virtual void setTargetSpeed(const real_t speed) = 0;
    virtual void setTargetPosition(const real_t pos) = 0;
    virtual void setTargetTrapezoid(const real_t pos) = 0;
    virtual void setOpenLoopCurrent(const real_t current) = 0;
    virtual void setTargetVector(const real_t pos) = 0;
    virtual void setCurrentClamp(const real_t max_current) = 0;
    virtual void locateRelatively(const real_t pos = 0) = 0;

    virtual bool isActive() const = 0;
    virtual const volatile RunStatus & status() = 0;

    virtual real_t getSpeed() const = 0;
    virtual real_t getPosition() const = 0;
    virtual real_t getCurrent() const = 0;

    virtual void setTargetPositionClamp(const Range & clamp) = 0;
    virtual void enable(const bool en = true) = 0;
    virtual void setNodeId(const uint8_t _id) = 0;
    virtual void setSpeedClamp(const real_t max_spd) = 0;
    virtual void setAccelClamp(const real_t max_acc) = 0;

    virtual void triggerCali() = 0;
    virtual void reset() = 0;
};