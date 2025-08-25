#pragma once

#include "robots/rpc/can_proto.hpp"

#include "stepper/archive/archive.hpp"
#include "stepper/ctrls/ctrls.hpp"

#include "drivers/Encoder/OdometerPoles.hpp"
#include "hal/bus/uart/uarthw.hpp"

namespace ymd::foc{
using namespace ymd::hal;
using namespace ymd::drivers;

class FOCMotorIntf{ 
public:
    // using namespace ymd::drivers;


    using ErrorCode = MotorUtils::ErrorCode;
    using RunStatus = MotorUtils::RunStatus;
    
    using CtrlType = MotorUtils::CtrlType;
    
    using NodeId = MotorUtils::NodeId;
    using MetaData = MotorUtils::MetaData;
    
protected:
    using ExitFlag = MotorUtils::ExitFlag;
    using InitFlag = MotorUtils::InitFlag;

    using Range = Range2<real_t>;
    using Switches = MotorUtils::Switches;


    volatile RunStatus run_status = RunStatus::INIT;

    MetaData meta;
    const uint8_t node_id;
    real_t elecrad_zerofix;

    friend class AsciiProtocol;
    friend class CanProtocol;
public:
    FOCMotorIntf(const NodeId _id):node_id(_id){;}
    


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

    virtual real_t getSpeed() const{return meta.spd;}
    virtual real_t getPosition() const {return meta.pos;}
    virtual real_t getTargetEstSpeed() const {return meta.targ_est_spd;}
    virtual real_t getCurrent() const {return meta.curr;}
    virtual real_t read_acc() const{return meta.acc;}

    virtual void setCurrentLimit(const real_t max_current) = 0;
    virtual void setPositionLimit(const Range & clamp) = 0;
    virtual void setSpeedLimit(const real_t max_spd) = 0;
    virtual void setAccLimit(const real_t max_acc) = 0;
    virtual void enable(const Enable en = EN) = 0;
    virtual void triggerCali() = 0;
    virtual void reset() = 0;

    virtual uint8_t getNodeId() {return node_id;}
    // virtual void setNodeId(const uint8_t _id){node_id = _id;}
    auto & getMeta(){return meta;}

    auto & id(){return node_id;}
    auto id() const {return node_id;}

};

class FOCMotor:public FOCMotorIntf {
protected:
    using Archive = MotorUtils::Archive;
    Archive archive_;
    
    ErrorCode error_code = ErrorCode::OK;

    const char * error_message = nullptr;
    const char * warn_message = nullptr;

    bool shutdown_when_error_occurred = true;
    bool shutdown_when_warn_occurred = true;

    void throw_error(const ErrorCode _error_code,const char * _error_message) {
        error_message = _error_message;
        status() = RunStatus::ERROR;
        if(shutdown_when_error_occurred){
            enable(DISEN);
        }
        // CLI_DEBUG(error_message);
    }

    void throw_warn(const ErrorCode ecode, const char * _warn_message){
        warn_message = _warn_message;
        status() = RunStatus::WARN;
        if(shutdown_when_warn_occurred){
            enable(DISEN);
        }
        // CLI_DEBUG(warn_message);
    }

    digipw::SVPWM & svpwm;
    OdometerPoles odo;
    Memory & memory;



public:
    class AsciiProtocol{
    protected:
        FOCMotor & motor;
        using Command = MotorUtils::Command;
        void parseArgs(const std::span<const StringView> args);
    public:
        AsciiProtocol(hal::UartHw & _logger, FOCMotor & _motor):motor(_motor){;}
    };

    friend class AsciiProtocol;


    class CanProtocol:public CanProtocolIntf{
    protected:
        using Command = MotorUtils::Command;
        FOCMotor & motor;

    public:
        CanProtocol(hal::Can & _can, FOCMotor & _motor):
            CanProtocolIntf(_can, _motor.id()),
            motor(_motor){;}

        void parseCanmsg(const CanMsg & msg) override;
    };

    friend class CanProtocol;


    AsciiProtocol * ascii_protocol;
    CanProtocol * can_protocol;

    FOCMotor(const NodeId _id, digipw::SVPWM & _svpwm, EncoderIntf & encoder, const size_t _poles, Memory & _memory):
            FOCMotorIntf(_id),
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
    virtual void setRadfix(const real_t _radfix) = 0;


    auto & archive(){
        return archive_;
    }
};

};