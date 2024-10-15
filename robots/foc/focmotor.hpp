#pragma once

#include "robots/rpc/ascii_proto.hpp"
#include "robots/rpc/can_proto.hpp"

#include "stepper/archive/archive.hpp"
#include "stepper/ctrls/ctrls.hpp"
#include "drivers/Encoder/odometer_poles.hpp"

class FOCMotorConcept{ 
public:
    using ErrorCode = MotorUtils::ErrorCode;
    using RunStatus = MotorUtils::RunStatus;
    using CtrlType = MotorUtils::CtrlType;
    using NodeId = MotorUtils::NodeId;
    using MetaData = MotorUtils::MetaData;
protected:
    using ExitFlag = MotorUtils::ExitFlag;
    using InitFlag = MotorUtils::InitFlag;

    using Range = Range_t<real_t>;

    using Switches = MotorUtils::Switches;


    volatile RunStatus run_status = RunStatus::INIT;

    MetaData meta;
    uint8_t node_id;
    real_t elecrad_zerofix;

    friend class AsciiProtocol;
    friend class CanProtocol;

    // NodeId getDefaultNodeId();
public:
    FOCMotorConcept(const NodeId _id):node_id(_id){;}
    


    virtual void freeze() = 0;
    virtual void setTargetCurrent(const real_t current) = 0;
    virtual void setTargetSpeed(const real_t speed) = 0;
    virtual void setTargetPosition(const real_t pos) = 0;
    virtual void setTargetPositionDelta(const real_t delta) = 0;
    virtual void setOpenLoopCurrent(const real_t current) = 0;
    virtual void setTargetVector(const real_t pos) = 0;
    virtual void setTargetTeach(const real_t max_curr) = 0;
    virtual void locateRelatively(const real_t pos = 0) = 0;

    virtual bool isActive() const = 0;
    virtual volatile RunStatus & status() = 0;

    const auto & getMeta() const {return meta;}

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
    virtual void setNodeId(const uint8_t _id){node_id = _id;}
    auto & getMeta(){return meta;}

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



public:
    class AsciiProtocol:public AsciiProtocolConcept{
    protected:
        FOCMotor & motor;
        using Command = MotorUtils::Command;
        void parseArgs(const Strings & args) override;
    public:
        AsciiProtocol(IOStream & _logger, FOCMotor & _motor):AsciiProtocolConcept(_logger), motor(_motor){;}
    };

    friend class AsciiProtocol;
    class CanProtocol:public CanProtocolConcept{
    protected:
        using Command = MotorUtils::Command;
        FOCMotor & motor;

    public:
        CanProtocol(Can & _can, FOCMotor & _motor):
            CanProtocolConcept(_can),
            motor(_motor){;}

        void parseCanmsg(const CanMsg & msg) override;
    };
    friend class CanProtocol;

    AsciiProtocol * ascii_protocol;
    CanProtocol * can_protocol;

    FOCMotor(const NodeId _id, SVPWM & _svpwm, Encoder & encoder, const size_t _poles, Memory & _memory):
            FOCMotorConcept(_id),
            svpwm(_svpwm), odo(encoder, _poles), memory(_memory){;}

    virtual bool loadArchive() = 0;
    virtual void saveArchive() = 0;
    virtual void removeArchive() = 0;

    void bindProtocol(AsciiProtocol & _ascii_protocol){
        ascii_protocol = &_ascii_protocol;
    }

    void bindProtocol(CanProtocol & _can_protocol){
        can_protocol = &_can_protocol;
    }

    const char * getErrMsg() const {
        return error_message;
    }

    const char * getWarnMsg() const {
        return warn_message;
    }

    virtual uint32_t exe() const = 0;
};