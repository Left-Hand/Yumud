#pragma once

#include "stepper/archive/archive.hpp"
#include "stepper/ctrls/ctrls.hpp"
#include "drivers/Encoder/odometer_poles.hpp"

#include "protocol/can_protocol.hpp"
#include "protocol/ascii_protocol.hpp"


class AsciiProtocol;
class CanProtocol;

class FOCMotorConcept{ 
public:
    using ErrorCode = MotorUtils::ErrorCode;
    using RunStatus = MotorUtils::RunStatus;
    using CtrlType = MotorUtils::CtrlType;
protected:
    using ExitFlag = MotorUtils::ExitFlag;
    using InitFlag = MotorUtils::InitFlag;

    using Range = Range_t<real_t>;

    using Switches = MotorUtils::Switches;
    using NodeId = MotorUtils::NodeId;


    volatile RunStatus run_status = RunStatus::INIT;
    NodeId node_id = 0;

    MetaData meta;
    real_t target;

    friend class AsciiProtocol;
    friend class CanProtocol;

    std::optional<AsciiProtocol> ascii_protocol;
    std::optional<CanProtocol> can_protocol;
public:
    FOCMotorConcept() : node_id(getDefaultNodeId()) {}
    FOCMotorConcept(NodeId _id) : node_id(_id) {}

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
    virtual volatile RunStatus & status() = 0;

    real_t getSpeed() const{return meta.spd;}

    real_t getPosition() const {return meta.pos;}

    real_t getCurrent() const {return meta.curr;}

    real_t getAccel() const{return meta.accel;}

    virtual void setCurrentLimit(const real_t max_current) = 0;
    virtual void setPositionLimit(const Range & clamp) = 0;
    virtual void setSpeedLimit(const real_t max_spd) = 0;
    virtual void setAccelLimit(const real_t max_acc) = 0;
    virtual void enable(const bool en = true) = 0;
    virtual void triggerCali() = 0;
    virtual void reset() = 0;

    virtual uint8_t getNodeId() {return node_id;}
    virtual void setNodeId(const NodeId _id){node_id = _id;}
private:
    uint8_t getDefaultNodeId();
};

class FOCMotor:public FOCMotorConcept {
protected:
    ErrorCode error_code = ErrorCode::OK;

    const char * error_message = nullptr;
    const char * warn_message = nullptr;

    bool shutdown_when_error_occurred = true;
    bool shutdown_when_warn_occurred = true;

    void throw_error(const ErrorCode _error_code,const char * _error_message) {
        error_message = _error_message;
        status() = RunStatus::ERROR;
        if(shutdown_when_error_occurred){
            enable(false);
        }
        // CLI_DEBUG(error_message);
    }

    void throw_warn(const ErrorCode ecode, const char * _warn_message){
        warn_message = _warn_message;
        status() = RunStatus::WARN;
        if(shutdown_when_warn_occurred){
            enable(false);
        }
        // CLI_DEBUG(warn_message);
    }

    SVPWM & svpwm;
    OdometerPoles odo;
    Memory & memory;


    friend class AsciiProtocol;
    friend class CanProtocol;

public:
    FOCMotor(SVPWM & _svpwm, Encoder & encoder, Memory & _memory):
            FOCMotorConcept(),
            svpwm(_svpwm), odo(encoder), memory(_memory){;}


    void bindProtocol(AsciiProtocol & _ascii_protocol){
        ascii_protocol.emplace(_ascii_protocol);
    }

    void bindProtocol(CanProtocol & _can_protocol){
        can_protocol.emplace(_can_protocol);
    }

    const char * getErrMsg() const {
        return error_message;
    }

    const char * getWarnMsg() const {
        return warn_message;
    }

    virtual uint32_t exe() const = 0;
};