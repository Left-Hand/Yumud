#pragma once

#include "stepper/concept.hpp"
#include "stepper/constants.hpp"
#include "stepper/cli.hpp"

class RemoteStepper:public StepperConcept{
protected:
    using ExitFlag = StepperEnums::ExitFlag;
    using InitFlag = StepperEnums::InitFlag;

    using Range = Range_t<real_t>;

    using RunStatus = StepperEnums::RunStatus;

    IOStream & logger;
    Can & can;
    uint8_t node_id;
    volatile RunStatus run_status = RunStatus::NONE;

    real_t spd;
    real_t pos;
    real_t curr;

    Range M_clamp{0,30};
public:
    RemoteStepper(IOStream & _logger, Can & _can, const uint8_t _node_id):
            logger(_logger), can(_can), node_id(_node_id){;}

    bool loadArchive(const bool outen);
    void saveArchive(const bool outen);
    void removeArchive(const bool outen);

    void setTargetCurrent(const real_t current);
    void setTargetSpeed(const real_t speed);
    void freeze();
    void setTargetPosition(const real_t pos);
    void setTargetTrapezoid(const real_t pos);
    void setOpenLoopCurrent(const real_t current);
    void setTargetVector(const real_t pos);
    void setCurrentClamp(const real_t max_current);
    void locateRelatively(const real_t pos = 0);

    bool isActive() const;
    const volatile RunStatus & status();

    real_t getSpeed() const;
    real_t getPosition() const;
    real_t getCurrent() const;

    void setPositionClamp(const Range & clamp);
    void enable(const bool en = true);
    void setNodeId(const uint8_t _id);
    void setSpeedClamp(const real_t max_spd);
    void setAccelClamp(const real_t max_acc);
    void reset();
    void triggerCali();

    void setNozzle(const real_t duty);
};