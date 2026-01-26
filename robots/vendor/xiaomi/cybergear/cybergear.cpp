#include "cybergear.hpp"

#include "core/utils/bits/bitfield_proxy.hpp"

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

using BxCanFrame = hal::BxCanFrame;
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


struct [[nodiscard]] [[nodiscard]] CgId final{


    [[nodiscard]] constexpr auto cmd() {return make_bitfield_proxy<24, 29, cybergear::Command>(bits_);}
    [[nodiscard]] constexpr auto cmd() const {return make_bitfield_proxy<24, 29, cybergear::Command>(bits_);}
    [[nodiscard]] constexpr auto high() {return make_bitfield_proxy<8, 24, uint16_t>(bits_);}
    [[nodiscard]] constexpr auto fault() {return make_bitfield_proxy<8, 24, uint16_t>(bits_);}
    [[nodiscard]] constexpr auto low() {return make_bitfield_proxy<0, 8, uint8_t>(bits_);}

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

    struct [[nodiscard]] CgPayload{
        uint64_t bits;
        [[nodiscard]] auto cmd_rad() {return make_bitfield_proxy<0, 16, CmdRad>(bits);}
        [[nodiscard]] auto cmd_omega() {return make_bitfield_proxy<16, 32, CmdOmega>(bits);}
        [[nodiscard]] auto cmd_kp() {return make_bitfield_proxy<32, 48, CmdKp>(bits);}
        [[nodiscard]] auto cmd_kd() {return make_bitfield_proxy<48, 64, CmdKd>(bits);}
    };

    CgPayload payload = {0};

    #define TRY_DESERIALIZE_PAYLOAD(class_name, field_name)\
    ({\
        const auto res = class_name::with_validation((field_name));\
        if(res.is_err()) return make_err_from_cmp(res.unwrap_err());\
        res.unwrap();\
    });\

    payload.cmd_rad() = TRY_DESERIALIZE_PAYLOAD(CmdRad, params.radians); 
    payload.cmd_omega() = TRY_DESERIALIZE_PAYLOAD(CmdOmega, params.omega);
    payload.cmd_kd() = TRY_DESERIALIZE_PAYLOAD(CmdKd, params.kd);
    payload.cmd_kp() = TRY_DESERIALIZE_PAYLOAD(CmdKp, params.kp);

    return this->transmit(
        CgId::from_parts(cybergear::Command::SEND_CTRL1, 
        CmdTorque(params.torque).count(), node_id_).to_bits(),
        payload.bits, sizeof(payload)
    );
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

IResult<> Self::transmit(const BxCanFrame & frame){
    MOTOR_DEBUG("write_msg", frame);
    TODO();
    // can_drv_.transmit(frame);
    return Ok{};
}

IResult<> Self::change_node_id(const uint8_t node_id){
    node_id_ = node_id;
    return this->transmit(
        CgId::from_parts(cybergear::Command::SET_CAN_ID, host_id_, node_id_).to_bits(),
            0, 0);
}

IResult<> Self::request_read_para(const uint16_t idx){
    return this->transmit(
        CgId::from_parts(cybergear::Command::READ_PARA, host_id_, node_id_).to_bits(), 
            uint64_t(idx), 8);
}

IResult<> Self::request_write_para(const uint16_t idx, const uint32_t bits){
    return this->transmit(
        CgId::from_parts(cybergear::Command::WRITE_PARA, host_id_, node_id_).to_bits(), 
        uint64_t(idx) | (uint64_t(bits) << 32), 
        8
    );
}

IResult<> Self::transmit(const uint32_t bits, const uint64_t payload, const uint8_t dlc){
    if (dlc > 8) 
        return Err(Error::RET_DLC_LONGER);

    const auto buf = std::bit_cast<std::array<uint8_t, 8>>(payload);
    const auto frame = BxCanFrame(
        hal::CanStdId::from_bits(bits), 
        hal::BxCanPayload::from_bytes(std::span(buf.data(), dlc))
    );
    return this->transmit(frame);
}

IResult<> Self::on_receive(const BxCanFrame & frame){
    if(!frame.is_extended())
        __builtin_trap();
    const auto id_u32 = frame.id_u32();
    const auto cgid = CgId::from_bits(id_u32);
    const auto cmd = cgid.cmd().get();

    const uint64_t bits = frame.payload_u64();
    const uint8_t dlc = frame.length();

    switch(cmd){
        case cybergear::Command::GET_DEVICE_ID:
            return on_mcu_id_feed_back(id_u32, bits, dlc);
        case cybergear::Command::FBK_CTRL1:
            return on_read_para_feed_back(id_u32, bits, dlc);
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

IResult<> Self::on_mcu_id_feed_back(const uint32_t id_u32, const uint64_t bits, const uint8_t dlc){
    if (dlc != 8){
        return Err(Error::RET_DLC_SHORTER);
    }

    device_mcu_id_ = Some(bits);
    return Ok();
}

struct [[nodiscard]] CgPayload{
    uint64_t bits;
    [[nodiscard]] constexpr auto radians() const {            
        return make_bitfield_proxy<0, 16, CmdRad>(bits);}
    [[nodiscard]] constexpr auto omega() const {          
        return make_bitfield_proxy<16, 32, CmdOmega>(bits);}
    [[nodiscard]] constexpr auto torque() const {         
        return make_bitfield_proxy<32, 48, CmdTorque>(bits);}
    [[nodiscard]] constexpr auto temperature() const {    
        return make_bitfield_proxy<48, 64, Temperature>(bits);}
};


IResult<> Self::on_ctrl2_feed_back(const uint32_t id_u32, const uint64_t bits, const uint8_t dlc){


    if(dlc != sizeof(CgPayload)){
        return Err(Error::RET_DLC_SHORTER);
    }

    const CgPayload payload = {bits};

    feedback_.radians =         payload.radians().get().to<real_t>();
    feedback_.omega =       payload.omega().get().to<real_t>();
    feedback_.torque =      payload.torque().get().to<real_t>();
    feedback_.temperature = static_cast<real_t>(payload.temperature().get());

    return Ok();
}