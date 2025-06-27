#include "MotorCyberGear.hpp"

#include "core/utils/BitField.hpp"

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
using namespace ymd::robots;
using namespace ymd::robots::details;

using CanMsg = hal::CanMsg;
using Temperature = CyberGear_Temperature;
using CmdRad = CyberGear_CmdRad;
using CmdOmega = CyberGear_CmdOmega;
using CmdTorque = CyberGear_CmdTorque;
using CmdKp = CyberGear_CmdKp;
using CmdKd = CyberGear_CmdKd;

template<typename T = void>
using IResult = Result<T, CyberGear_Error>;

struct CgId final{
    uint32_t id;

    constexpr auto cmd() {return make_bitfield<24, 29, CyberGear_Command>(id);}
    constexpr auto cmd() const {return make_bitfield<24, 29, CyberGear_Command>(id);}
    constexpr auto high() {return make_bitfield<8, 24, uint16_t>(id);}
    constexpr auto fault() {return make_bitfield<8, 24, uint16_t>(id);}
    constexpr auto low() {return make_bitfield<0, 8, uint8_t>(id);}

    static constexpr 
    CgId from(const CyberGear_Command cmd, const uint16_t high, const uint8_t low) {
        CgId self;

        self.cmd() = cmd;
        self.high() = high;
        self.low() = low;

        return self;
    }


};





IResult<> CyberGear::init(){
    return Ok{};
}

IResult<> CyberGear::request_mcu_id(){
    const auto id = CgId::from(CyberGear_Command::GET_DEVICE_ID, host_id_, node_id_).id;
    return this->transmit(id, 0, 0);
}

IResult<> CyberGear::ctrl(const MitParams & params){

    struct Payload{
        uint64_t data;
        auto cmd_rad() {return make_bitfield<0, 16, CmdRad>(data);}
        auto cmd_omega() {return make_bitfield<16, 32, CmdOmega>(data);}
        auto cmd_kp() {return make_bitfield<32, 48, CmdKp>(data);}
        auto cmd_kd() {return make_bitfield<48, 64, CmdKd>(data);}
    };

    Payload payload = {0};

    {
        const auto res = CmdTorque::check(params.torque) 
            | CmdRad::check(params.rad) 
            | CmdOmega::check(params.omega) 
            | CmdKp::check(params.kp) 
            | CmdKd::check(params.kd) 
        ;

        if(res) return Err(CyberGear_Error::INPUT_OUT_OF_RANGE);
    }

    payload.cmd_rad() = CmdRad(params.rad);
    payload.cmd_omega() = CmdOmega(params.omega);
    payload.cmd_kd() = CmdKd(params.kd);
    payload.cmd_kp() = CmdKp(params.kp);

    return this->transmit(
        CgId::from(CyberGear_Command::SEND_CTRL1, 
        CmdTorque(params.torque).data, node_id_).id,
        payload.data, sizeof(payload)
    );
}

IResult<> CyberGear::on_mcu_id_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc){
    if (dlc != 8){
        return Err(CyberGear_Error::RET_DLC_SHORTER);
    }

    device_mcu_id_ = Some(data);
    return Ok();
}

IResult<> CyberGear::on_ctrl2_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc){
    struct Payload{
        uint64_t data;
        auto rad() {            return make_bitfield<0, 16, CmdRad>(data);}
        auto omega() {          return make_bitfield<16, 32, CmdOmega>(data);}
        auto torque() {         return make_bitfield<32, 48, CmdTorque>(data);}
        auto temperature() {    return make_bitfield<48, 64, Temperature>(data);}
    };


    if(dlc != sizeof(Payload)){
        return Err(CyberGear_Error::RET_DLC_SHORTER);
    }

    Payload payload = {data};

    feedback_.rad = 
        real_t(payload.rad().as_val());
    feedback_.omega = 
        real_t(payload.omega().as_val());
    feedback_.torque = 
        real_t(payload.torque().as_val());
    feedback_.temperature = 
        real_t(payload.temperature().as_val());

    return Ok();
}

IResult<> CyberGear::enable(const Enable en, const bool clear_fault){
    if(en == EN){
        return this->transmit(
            CgId::from(CyberGear_Command::EN_MOT, host_id_, node_id_).id, 0, 0);
    }else{
        // 正常运行时，data区需清0；
        // byte[0]=1 时：清故障；
        return this->transmit(
            CgId::from(CyberGear_Command::DISEN_MOT, host_id_, node_id_).id, 
            uint64_t(clear_fault), 8);
    }
}


IResult<> CyberGear::set_current_as_machine_home(){
    return this->transmit(
        CgId::from(CyberGear_Command::SET_MACHINE_HOME, host_id_, node_id_).id, 1, 0);
}

IResult<> CyberGear::transmit(const CanMsg & msg){
    MOTOR_DEBUG("write_msg", msg);
    // can_drv_.transmit(msg);
    return Ok{};
}

IResult<> CyberGear::change_can_id(const uint8_t can_id){
    node_id_ = can_id;
    return this->transmit(
        CgId::from(CyberGear_Command::SET_CAN_ID, host_id_, node_id_).id,
            0, 0);
}

IResult<> CyberGear::request_read_para(const uint16_t idx){
    return this->transmit(
        CgId::from(CyberGear_Command::READ_PARA, host_id_, node_id_).id, 
            uint64_t(idx), 8);
}

IResult<> CyberGear::request_write_para(const uint16_t idx, const uint32_t data){
    return this->transmit(
        CgId::from(CyberGear_Command::WRITE_PARA, host_id_, node_id_).id, 
        uint64_t(idx) | (uint64_t(data) << 32), 
        8
    );
}

IResult<> CyberGear::transmit(const uint32_t id, const uint64_t payload, const uint8_t dlc){
    if (dlc > 8) 
        return Err(CyberGear_Error::RET_DLC_LONGER);

    const auto buf = std::bit_cast<std::array<uint8_t, 8>>(payload);
    const auto msg = CanMsg::from_bytes(
        hal::CanStdId(id), 
        std::span(buf.data(), dlc)
    );
    return this->transmit(msg);
}

IResult<> CyberGear::on_receive(const CanMsg & msg){
    const auto id = msg.stdid().unwrap().as_raw();
    const auto cgid = CgId{id};
    const auto cmd = cgid.cmd().as_val();

    const uint64_t data = msg.payload_as_u64();
    const uint8_t dlc = msg.size();

    switch(cmd){
        case CyberGear_Command::GET_DEVICE_ID:
            return on_mcu_id_feed_back(id, data, dlc);
        case CyberGear_Command::FBK_CTRL1:
            return on_read_para_feed_back(id, data, dlc);
        case CyberGear_Command::READ_PARA:
            return Err{CyberGear_Error::PRAGRAM_TODO};
            break;
        case CyberGear_Command::FBK_FAULT:
            return Err{CyberGear_Error::PRAGRAM_TODO};
            break;

        default:
            return Err{CyberGear_Error::RET_UNKOWN_CMD};
    }

    return Err{CyberGear_Error::PRAGRAM_UNHANDLED};
}