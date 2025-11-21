#include "MotorCyberGear.hpp"

#include "core/utils/bits/BitField.hpp"

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
using namespace ymd::robots::cybergear::details;

using CanClassicFrame = hal::CanClassicFrame;
using Temperature = cybergear::details::Temperature;
using CmdRad = cybergear::details::CmdRad;
using CmdOmega = cybergear::details::CmdOmega;
using CmdTorque = cybergear::details::CmdTorque;
using CmdKp = cybergear::details::CmdKp;
using CmdKd = cybergear::details::CmdKd;

using Self = cybergear::CyberGear;

using Error = cybergear::Error;
template<typename T = void>
using IResult = Result<T, Error>;

static constexpr Err<Error> make_err_from_cmp(const std::weak_ordering ord){
    if (ord == std::weak_ordering::less) {
        return Err<Error>(Error::INPUT_LOWER_THAN_LIMIT);
    } else if (ord == std::weak_ordering::greater) {
        return Err<Error>(Error::INPUT_HIGHER_THAN_LIMIT);
    } else {
        __builtin_unreachable();
    }
}


struct [[nodiscard]] CgId final{


    [[nodiscard]] constexpr auto cmd() {return make_bitfield<24, 29, cybergear::Command>(bits_);}
    [[nodiscard]] constexpr auto cmd() const {return make_bitfield<24, 29, cybergear::Command>(bits_);}
    [[nodiscard]] constexpr auto high() {return make_bitfield<8, 24, uint16_t>(bits_);}
    [[nodiscard]] constexpr auto fault() {return make_bitfield<8, 24, uint16_t>(bits_);}
    [[nodiscard]] constexpr auto low() {return make_bitfield<0, 8, uint8_t>(bits_);}

    [[nodiscard]] static constexpr 
    CgId from_parts(const cybergear::Command cmd, const uint16_t high, const uint8_t low) {
        CgId self;

        self.cmd() = cmd;
        self.high() = high;
        self.low() = low;

        return self;
    }

    [[nodiscard]] static constexpr CgId from_bits(const uint32_t bits) {
        CgId self;
        self.bits_ = bits;
        return self;
    }

    [[nodiscard]] constexpr uint32_t to_bits() const {return bits_;}

private:
    uint32_t bits_;

};



IResult<> Self::init(){
    return Ok{};
}

IResult<> Self::request_mcu_id(){
    const auto bits = CgId::from_parts(cybergear::Command::GET_DEVICE_ID, host_id_, node_id_).to_bits();
    return this->transmit(bits, 0, 0);
}

IResult<> Self::ctrl(const MitParams & params){

    struct CgPayload{
        uint64_t data;
        auto cmd_rad() {return make_bitfield<0, 16, CmdRad>(data);}
        auto cmd_omega() {return make_bitfield<16, 32, CmdOmega>(data);}
        auto cmd_kp() {return make_bitfield<32, 48, CmdKp>(data);}
        auto cmd_kd() {return make_bitfield<48, 64, CmdKd>(data);}
    };

    CgPayload payload = {0};

    #define TRY_DESERIALIZE_PAYLOAD(class_name, field_name)\
    ({\
        const auto res = class_name::with_validation((field_name));\
        if(res.is_err()) return make_err_from_cmp(res.unwrap_err());\
        res.unwrap();\
    });\

    payload.cmd_rad() = TRY_DESERIALIZE_PAYLOAD(CmdRad, params.rad); 
    payload.cmd_omega() = TRY_DESERIALIZE_PAYLOAD(CmdOmega, params.omega);
    payload.cmd_kd() = TRY_DESERIALIZE_PAYLOAD(CmdKd, params.kd);
    payload.cmd_kp() = TRY_DESERIALIZE_PAYLOAD(CmdKp, params.kp);

    return this->transmit(
        CgId::from_parts(cybergear::Command::SEND_CTRL1, 
        CmdTorque(params.torque).count(), node_id_).to_bits(),
        payload.data, sizeof(payload)
    );
}

IResult<> Self::on_mcu_id_feed_back(const uint32_t bits, const uint64_t data, const uint8_t dlc){
    if (dlc != 8){
        return Err(Error::RET_DLC_SHORTER);
    }

    device_mcu_id_ = Some(data);
    return Ok();
}

struct CgPayload{
    uint64_t data;
    constexpr auto rad() const {            
        return make_bitfield<0, 16, CmdRad>(data);}
    constexpr auto omega() const {          
        return make_bitfield<16, 32, CmdOmega>(data);}
    constexpr auto torque() const {         
        return make_bitfield<32, 48, CmdTorque>(data);}
    constexpr auto temperature() const {    
        return make_bitfield<48, 64, Temperature>(data);}
};


IResult<> Self::on_ctrl2_feed_back(const uint32_t bits, const uint64_t data, const uint8_t dlc){


    if(dlc != sizeof(CgPayload)){
        return Err(Error::RET_DLC_SHORTER);
    }

    const CgPayload payload = {data};

    feedback_.rad =         payload.rad().to_bits().to<real_t>();
    feedback_.omega =       payload.omega().to_bits().to<real_t>();
    feedback_.torque =      payload.torque().to_bits().to<real_t>();
    feedback_.temperature = static_cast<real_t>(payload.temperature().to_bits());

    return Ok();
}

IResult<> Self::enable(const Enable en, const bool clear_fault){
    if(en == EN){
        return this->transmit(
            CgId::from_parts(cybergear::Command::EN_MOT, host_id_, node_id_).to_bits(), 0, 0);
    }else{
        // 正常运行时，data区需清0；
        // byte[0]=1 时：清故障；
        return this->transmit(
            CgId::from_parts(cybergear::Command::DISEN_MOT, host_id_, node_id_).to_bits(), 
            uint64_t(clear_fault), 8);
    }
}


IResult<> Self::set_current_as_machine_home(){
    return this->transmit(
        CgId::from_parts(cybergear::Command::SET_MACHINE_HOME, host_id_, node_id_).to_bits(), 1, 0);
}

IResult<> Self::transmit(const CanClassicFrame & frame){
    MOTOR_DEBUG("write_msg", frame);
    // can_drv_.transmit(frame);
    return Ok{};
}

IResult<> Self::change_can_id(const uint8_t can_id){
    node_id_ = can_id;
    return this->transmit(
        CgId::from_parts(cybergear::Command::SET_CAN_ID, host_id_, node_id_).to_bits(),
            0, 0);
}

IResult<> Self::request_read_para(const uint16_t idx){
    return this->transmit(
        CgId::from_parts(cybergear::Command::READ_PARA, host_id_, node_id_).to_bits(), 
            uint64_t(idx), 8);
}

IResult<> Self::request_write_para(const uint16_t idx, const uint32_t data){
    return this->transmit(
        CgId::from_parts(cybergear::Command::WRITE_PARA, host_id_, node_id_).to_bits(), 
        uint64_t(idx) | (uint64_t(data) << 32), 
        8
    );
}

IResult<> Self::transmit(const uint32_t bits, const uint64_t payload, const uint8_t dlc){
    if (dlc > 8) 
        return Err(Error::RET_DLC_LONGER);

    const auto buf = std::bit_cast<std::array<uint8_t, 8>>(payload);
    const auto frame = CanClassicFrame(
        hal::CanStdId::from_bits(bits), 
        hal::CanClassicPayload::from_bytes(std::span(buf.data(), dlc))
    );
    return this->transmit(frame);
}

IResult<> Self::on_receive(const CanClassicFrame & frame){
    if(!frame.is_extended())
        __builtin_trap();
    const auto bits = frame.identifier().to_bits();
    const auto cgid = CgId::from_bits(bits);
    const auto cmd = cgid.cmd().to_bits();

    const uint64_t data = frame.payload_u64();
    const uint8_t dlc = frame.length();

    switch(cmd){
        case cybergear::Command::GET_DEVICE_ID:
            return on_mcu_id_feed_back(bits, data, dlc);
        case cybergear::Command::FBK_CTRL1:
            return on_read_para_feed_back(bits, data, dlc);
        case cybergear::Command::READ_PARA:
            return Err{Error::PRAGRAM_TODO};
            break;
        case cybergear::Command::FBK_FAULT:
            return Err{Error::PRAGRAM_TODO};
            break;

        default:
            return Err{Error::RET_UNKOWN_CMD};
    }

    return Err{Error::PRAGRAM_UNHANDLED};
}