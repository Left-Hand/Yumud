#pragma once

#include "robots/foc/focmotor.hpp"
#include "robots/foc/stepper/motor_utils.hpp"

class RemoteFOCMotor:public FOCMotorConcept, public CanProtocolConcept{

protected:
    using ExitFlag = MotorUtils::ExitFlag;
    using InitFlag = MotorUtils::InitFlag;
    using RunStatus = MotorUtils::RunStatus;
    
    using Command = MotorUtils::Command;
    volatile RunStatus run_status = RunStatus::NONE;

public:
    // class CanProtocolRemote:CanProtocolConcept{
    // protected:
    //     using Command = MotorUtils::Command;
    //     RemoteFOCMotor & motor;
    // public:
    //     CanProtocolRemote(Can & _can, RemoteFOCMotor & _motor):
    //         CanProtocolConcept(_can), motor(_motor){;}

    void parseCanmsg(const CanMsg & msg) override;
    // };

    IOStream & logger;

    using E = CanProtocolConcept::E;
    using E_2 = std::tuple<E, E>;
    using E_3 = std::tuple<E, E, E>;
    using E_4 = std::tuple<E, E, E, E>;
public:
    RemoteFOCMotor(IOStream & _logger, Can & _can, NodeId _id):
            CanProtocolConcept(_can, _id), logger(_logger){;}

    bool loadArchive();
    void saveArchive();
    void removeArchive();

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
    real_t getTarget() override {return 0;}
    bool isActive() const;
    volatile RunStatus & status();

    real_t getSpeed() const;
    real_t getPosition() const;
    real_t getCurrent() const;
    real_t getAccel() const;
    real_t readSpeed() const{return meta.spd;}
    real_t readPosition() const{return meta.pos;}
    real_t readCurrent() const{return meta.curr;}
    real_t readAccel() const{return meta.accel;}
    void updateAll() const;

    void setPositionLimit(const Range & clamp);
    void enable(const bool en = true);
    // void setNodeId(const NodeId _id);
    void setSpeedLimit(const real_t max_spd);
    void setAccelLimit(const real_t max_acc);
    void reset();
    void triggerCali();


    void setNozzle(const real_t duty);
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