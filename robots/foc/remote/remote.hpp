#pragma once

#include "robots/foc/focmotor.hpp"
#include "robots/foc/motor_utils.hpp"


namespace ymd::foc{
class RemoteFOCMotor:public FOCMotorConcept, public CanProtocolConcept{

protected:
    using ExitFlag = MotorUtils::ExitFlag;
    using InitFlag = MotorUtils::InitFlag;
    using RunStatus = MotorUtils::RunStatus;
    using NodeId = MotorUtils::NodeId;
    
    using Command = MotorUtils::Command;
    volatile RunStatus run_status = RunStatus::NONE;

protected:

    void parseCanmsg(const CanMsg & msg) override;
    
    IOStream & logger;
    
    using CanProtocolConcept::E;
    using CanProtocolConcept::E_2;
    using CanProtocolConcept::E_3;
    using CanProtocolConcept::E_4;
public:
    RemoteFOCMotor(const RemoteFOCMotor & other) = delete;
    RemoteFOCMotor(RemoteFOCMotor && other) = delete;

    RemoteFOCMotor(IOStream & _logger, Can & _can, NodeId _id):
            FOCMotorConcept(_id), CanProtocolConcept(_can, _id), logger(_logger){;}

            
    void setTargetCurrent(const real_t current);
    void setTargetSpeed(const real_t speed);
    void freeze();
    void setTargetPosition(const real_t pos);
    void setTargetPositionDelta(const real_t delta);
    void setOpenLoopCurrent(const real_t current);
    void setTargetTeach(const real_t current);
    void setTargetVector(const real_t pos);
    void setCurrentLimit(const real_t max_current);
    void locateRelatively(const real_t pos = 0);
    bool isActive() const;
    volatile RunStatus & status();

    real_t getSpeed() const override;
    real_t getPosition() const override;
    real_t getCurrent() const override;
    real_t getAcc() const override;
    real_t readSpeed() const{return meta.spd;}
    real_t readPosition() const{return meta.pos;}
    real_t readCurrent() const{return meta.curr;}
    real_t readAcc() const{return meta.acc;}
    void updateAll() const;

    void setPositionLimit(const Range & clamp);
    void enable(const bool en = true);
    // void setNodeId(const NodeId _id);
    void setSpeedLimit(const real_t max_spd);
    void setAccLimit(const real_t max_acc);
    void reset();
    void triggerCali();
    // void parseCan(const CanMsg & msg);
    // void parseCommand(const NodeId id, const Command cmd, const CanMsg &msg);

    uint8_t getNodeId() {return 0;}
    bool stable() const {
        return ABS(readSpeed()) < real_t(0.07);
    }
    bool reached(const real_t targ_pos) const {
        return (ABS(readPosition() - targ_pos) < real_t(0.2)) and stable();
    }
};

}