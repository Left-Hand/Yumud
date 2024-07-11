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
    volatile RunStatus run_status = RunStatus::NONE;
    uint8_t node_id;

    real_t spd;
    real_t pos;
    real_t curr;
public:
    RemoteStepper(IOStream & _logger, Can & _can, const uint8_t _node_id):logger(_logger), can(_can), node_id(_node_id){;}

    bool loadArchive(const bool outen);
    void saveArchive(const bool outen);
    void removeArchive(const bool outen);
    virtual bool autoload(const bool outen);

    void setTargetCurrent(const real_t current);
    void setTargetSpeed(const real_t speed);
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

    void setTargetPositionClamp(const Range & clamp);
    void enable(const bool en = true);
    void setNodeId(const uint8_t _id);
    void setSpeedClamp(const real_t max_spd);
    void setAccelClamp(const real_t max_acc);
    void reset();
    void triggerCali();

    void mt(const bool en);
};

struct RemoteSteppers{
public:
    RemoteStepper & w;
    RemoteStepper & x;
    RemoteStepper & y;
    RemoteStepper & z;
    RemoteSteppers(
        RemoteStepper & _w,
        RemoteStepper & _x,
        RemoteStepper & _y,
        RemoteStepper & _z
    ):w(_w), x(_x), y(_y), z(_z){;}

    RemoteStepper & operator [](const uint8_t index){
        switch(index){
            case 0:
                return w;
            case 1:
                return x;
            case 2:
                return y;
            case 3:
                return z;
            default:
                return w;
        }
    }
};