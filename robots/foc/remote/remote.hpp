#pragma once

#include "robots/foc/focmotor.hpp"
#include "robots/foc/stepper/constants.hpp"
#include "robots/foc/stepper/cli.hpp"

#include "robots/foc/stepper/protocol/can_protocol.hpp"

class RemoteFOCMotor:public FOCMotor{
protected:
    using ExitFlag = StepperEnums::ExitFlag;
    using InitFlag = StepperEnums::InitFlag;
    using NodeId = StepperUtils::NodeId;
    using RunStatus = StepperEnums::RunStatus;

    using Command = StepperEnums::Command;
    IOStream & logger;
    Can & can;
    uint8_t node_id;
    volatile RunStatus run_status = RunStatus::NONE;
public:
    RemoteFOCMotor(IOStream & _logger, Can & _can, const NodeId _node_id):
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
    void setTargetTeach(const real_t current);
    void setTargetVector(const real_t pos);
    void setCurrentLimit(const real_t max_current);
    void locateRelatively(const real_t pos = 0);

    bool isActive() const;
    const volatile RunStatus & status();

    real_t getSpeed() const;
    real_t getPosition() const;
    real_t getCurrent() const;
    real_t getAccel() const;
    real_t readSpeed() const{return measurements.spd;}
    real_t readPosition() const{return measurements.pos;}
    real_t readCurrent() const{return measurements.curr;}
    real_t readAccel() const{return measurements.accel;}
    void updateAll() const;

    void setPositionLimit(const Range & clamp);
    void enable(const bool en = true);
    void setNodeId(const NodeId _id);
    void setSpeedLimit(const real_t max_spd);
    void setAccelLimit(const real_t max_acc);
    void reset();
    void triggerCali();


    void setNozzle(const real_t duty);
    void parseCan(const CanMsg & msg);
    void parseCommand(const NodeId id, const Command cmd, const CanMsg &msg);

    bool stable() const {
        return ABS(readSpeed()) < real_t(0.07);
    }
    bool reached(const real_t targ_pos) const {
        return (ABS(readPosition() - targ_pos) < real_t(0.2)) and stable();
    }
};