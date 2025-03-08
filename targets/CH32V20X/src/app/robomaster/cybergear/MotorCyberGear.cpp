#include "MotorCyberGear.hpp"

#include "sys/utils/BitField.hpp"

#define MOTOR_DEBUG_EN

#ifdef MOTOR_DEBUG_EN
#define MOTOR_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MOTOR_PANIC(...) PANIC{__VA_ARGS__}
#define MOTOR_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define MOTOR_DEBUG(...)
#define MOTOR_PANIC(...)  PANIC_NSRC()
#define MOTOR_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd;
using namespace ymd::rmst;
using namespace ymd::rmst::details;

using CanMsg = hal::CanMsg;

// using CgResult = MotorCyberGear::CgResult;  
// using CgError = MotorCyberGear::CgError;



struct CgId{
    uint32_t id;

    auto cmd() {return make_bitfield<24, 29, CgCommand>(id);}
    auto cmd() const {return make_bitfield<24, 29, CgCommand>(id);}
    auto high() {return make_bitfield<8, 24, uint16_t>(id);}
    auto fault() {return make_bitfield<8, 24, uint16_t>(id);}
    auto low() {return make_bitfield<0, 8, uint8_t>(id);}
};



static __inline 
uint32_t make_id(const CgCommand cmd, const uint16_t high, const uint8_t low) {
    CgId cgid;
    cgid.cmd() = cmd;
    cgid.high() = high;
    cgid.low() = low;

    return cgid.id;
}



CgResult<void> MotorCyberGear::init(){
    return Ok{};
}

CgResult<void> MotorCyberGear::requestMcuId(){
    const auto id = make_id(CgCommand::GET_DEVICE_ID, host_id_, node_id_);
    return this->transmit(id, 0, 0);
}

CgResult<void> MotorCyberGear::ctrl(const real_t cmd_torque, const real_t cmd_rad, const real_t cmd_omega, const real_t cmd_kp, const real_t cmd_kd){

    struct Payload{
        uint64_t data;
        auto cmd_rad() {return make_bitfield<0, 16, CmdRad>(data);}
        auto cmd_omega() {return make_bitfield<16, 32, CmdOmega>(data);}
        auto cmd_kp() {return make_bitfield<32, 48, CmdKp>(data);}
        auto cmd_kd() {return make_bitfield<48, 64, CmdKd>(data);}
    };

    Payload payload = {0};

    {
        const auto res = 
            CmdTorque::check(cmd_torque) |
            CmdRad::check(cmd_rad) | 
            CmdOmega::check(cmd_omega) |
            CmdKp::check(cmd_kp) |
            CmdKd::check(cmd_kd) 
        ;

        if(res) return Err(CgError::INPUT_OUT_OF_RANGE);
    }

    payload.cmd_rad() = CmdRad(cmd_rad);
    payload.cmd_omega() = CmdOmega(cmd_omega);
    payload.cmd_kd() = CmdKd(cmd_kd);
    payload.cmd_kp() = CmdKp(cmd_kp);

    return this->transmit(
        make_id(CgCommand::SEND_CTRL1, CmdTorque(cmd_torque).data, node_id_),
        payload.data, sizeof(payload)
    );
}

CgResult<void> MotorCyberGear::onMcuIdFeedBack(const uint32_t id, const uint64_t data, const uint8_t dlc){
    if (dlc != 8){
        return Err(CgError::RET_DLC_SHORTER);
    }

    device_mcu_id_ = Some(data);
    return Ok();
}

CgResult<void> MotorCyberGear::onCtrl2FeedBack(const uint32_t id, const uint64_t data, const uint8_t dlc){
    struct Payload{
        uint64_t data;
        auto rad() {return make_bitfield<0, 16, CmdRad>(data);}
        auto omega() {return make_bitfield<16, 32, CmdOmega>(data);}
        auto torque() {return make_bitfield<32, 48, CmdTorque>(data);}
        auto temperature() {return make_bitfield<48, 64, Temperature>(data);}
    };


    if(dlc != sizeof(Payload)){
        return Err(CgError::RET_DLC_SHORTER);
    }

    Payload payload = {data};

    feedback_.rad = real_t(payload.rad());
    feedback_.omega = real_t(payload.omega());
    feedback_.torque = real_t(payload.torque());
    feedback_.temperature = real_t(payload.temperature());

    return Ok();
}

CgResult<void> MotorCyberGear::enable(const bool en, const bool clear_fault){
    if(en){
        return this->transmit(
            make_id(CgCommand::EN_MOT, host_id_, node_id_), 0, 0);
    }else{
        // 正常运行时，data区需清0；
        // byte[0]=1 时：清故障；
        return this->transmit(
            make_id(CgCommand::DISEN_MOT, host_id_, node_id_), uint64_t(clear_fault), 8);
    }
}


CgResult<void> MotorCyberGear::setCurrentAsMachineHome(){
    return this->transmit(
        make_id(CgCommand::SET_MACHINE_HOME, host_id_, node_id_), 1, 0);
}

CgResult<void> MotorCyberGear::transmit(const CanMsg & msg){
    MOTOR_DEBUG("write_msg", msg);
    // can_drv_.transmit(msg);
    return Ok{};
}

CgResult<void> MotorCyberGear::changeCanId(const uint8_t can_id){
    node_id_ = can_id;
    return this->transmit(
        make_id(CgCommand::SET_CAN_ID, host_id_, node_id_), 0, 0);
}

CgResult<void> MotorCyberGear::requestReadPara(const uint16_t idx){
    return this->transmit(
        make_id(CgCommand::READ_PARA, host_id_, node_id_), uint64_t(idx), 8);
}

CgResult<void> MotorCyberGear::requestWritePara(const uint16_t idx, const uint32_t data){
    return this->transmit(
        make_id(CgCommand::WRITE_PARA, host_id_, node_id_), uint64_t(idx) | (uint64_t(data) << 32), 8);
}

CgResult<void> MotorCyberGear::transmit(const uint32_t id, const uint64_t payload, const uint8_t dlc){
    if (dlc > 8) return CgResult<void>(CgError::RET_DLC_LONGER);
    const auto msg = CanMsg(id, payload, dlc);
    return this->transmit(msg);
}

CgResult<void> MotorCyberGear::onReceive(const CanMsg & msg){
    const auto id = msg.id();
    const auto cgid = CgId{id};
    const auto cmd = cgid.cmd().as_val();

    const uint64_t data = msg.data64();
    const uint8_t dlc = msg.size();



    switch(cmd){
        case CgCommand::GET_DEVICE_ID:
            return onMcuIdFeedBack(id, data, dlc);
        case CgCommand::FBK_CTRL1:
            return onReadParaFeedBack(id, data, dlc);
        case CgCommand::READ_PARA:
            return Err{CgError::PRAGRAM_TODO};
            break;
        case CgCommand::FBK_FAULT:
            return Err{CgError::PRAGRAM_TODO};
            break;

        default:
            return Err{CgError::RET_UNKOWN_CMD};
    }
    return Err{CgError::PRAGRAM_UNHANDLED};
}