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

using ClassicCanFrame = hal::ClassicCanFrame;
using TemperatureCode = cybergear::details::TemperatureCode;
using RadCode = cybergear::details::RadCode;
using OmegaCode = cybergear::details::OmegaCode;
using TorqueCode = cybergear::details::TorqueCode;
using KpCode = cybergear::details::KpCode;
using KdCode = cybergear::details::KdCode;

using Error = cybergear::Error;
namespace{
struct alignas(4) [[nodiscard]] CgId final{

    uint32_t bits;


    template <typename Self>
    [[nodiscard]] constexpr auto cmd(this Self && self) {
        return make_bitfield_proxy<24, 29, cybergear::Command>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto high(this Self && self) {
        return make_bitfield_proxy<8, 24, uint16_t>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto fault(this Self && self) {
        return make_bitfield_proxy<8, 24, uint16_t>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto low(this Self && self) {
        return make_bitfield_proxy<0, 8, uint8_t>(&self.bits);}

    [[nodiscard]] static constexpr 
    CgId from_parts(const cybergear::Command cmd, const uint16_t high, const uint8_t low) {
        CgId self;

        self.cmd().set(cmd);
        self.high().set(high);
        self.low().set(low);

        return self;
    }

    [[nodiscard]] static constexpr CgId from_bits(const uint32_t bits) {
        CgId self;
        self.bits = bits;
        return self;
    }

    [[nodiscard]] constexpr uint32_t to_bits() const noexcept {return bits;}

    [[nodiscard]] constexpr hal::CanExtId to_extid() const noexcept {return hal::CanExtId::from_bits(bits);}
};


struct alignas(4)  [[nodiscard]] TxContext final{
    uint64_t bits;

    template <typename Self>
    [[nodiscard]] constexpr auto cmd_rad(this Self && self) {
        return make_bitfield_proxy<0, 16, RadCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto cmd_omega(this Self && self) {
        return make_bitfield_proxy<16, 32, OmegaCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto cmd_kp(this Self && self) {
        return make_bitfield_proxy<32, 48, KpCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto cmd_kd(this Self && self) {
        return make_bitfield_proxy<48, 64, KdCode>(&self.bits);}

    constexpr hal::ClassicCanPayload to_can_payload() const noexcept {return hal::ClassicCanPayload::from_u64(bits);}
};

static_assert(sizeof(TxContext) == 8);  

struct alignas(4)  [[nodiscard]] RxContext final{
    uint64_t bits;

    template <typename Self>
    [[nodiscard]] constexpr auto radians(this Self && self) {            
        return make_bitfield_proxy<0, 16, RadCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto omega(this Self && self) {          
        return make_bitfield_proxy<16, 32, OmegaCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto torque(this Self && self) {         
        return make_bitfield_proxy<32, 48, TorqueCode>(&self.bits);}

    template <typename Self>
    [[nodiscard]] constexpr auto temperature(this Self && self) {    
        return make_bitfield_proxy<48, 64, TemperatureCode>(&self.bits);}
};

static_assert(sizeof(RxContext) == 8);


static constexpr Err<Error> make_err_from_cmp(const std::weak_ordering ord){
    if (ord == std::weak_ordering::less) {
        return Err<Error>(Error::INPUT_LOWER_THAN_LIMIT);
    } else if (ord == std::weak_ordering::greater) {
        return Err<Error>(Error::INPUT_HIGHER_THAN_LIMIT);
    } else {
        __builtin_unreachable();
    }
}

}


template<typename T = void>
using IResult = Result<T, Error>;


namespace ymd::robots::cybergear{
hal::ClassicCanFrame CyberGearFactory::request_mcu_id(){
    const auto extid = CgId::from_parts(
        cybergear::Command::GET_DEVICE_ID, host_id, node_id).to_extid();

    return hal::ClassicCanFrame::from_empty_data(
        extid
    );
}

hal::ClassicCanFrame CyberGearFactory::ctrl(const MitParams & params){
    TxContext tx_context = {0};

    #define TRY_DESERIALIZE_PAYLOAD(class_name, field_name)\
    ({\
        const auto res = class_name::with_validation((field_name));\
        if(res.is_err()) PANIC(res.unwrap_err());\
        res.unwrap();\
    });\

    tx_context.cmd_rad() = TRY_DESERIALIZE_PAYLOAD(RadCode, params.radians); 
    tx_context.cmd_omega() = TRY_DESERIALIZE_PAYLOAD(OmegaCode, params.omega);
    tx_context.cmd_kd() = TRY_DESERIALIZE_PAYLOAD(KdCode, params.kd);
    tx_context.cmd_kp() = TRY_DESERIALIZE_PAYLOAD(KpCode, params.kp);

    const auto extid = CgId::from_parts(
        cybergear::Command::SEND_CTRL1, 
        TorqueCode(params.torque).count(), 
        node_id).to_extid();

    return hal::ClassicCanFrame::from_parts(
        extid,
        tx_context.to_can_payload()
    );
}



hal::ClassicCanFrame CyberGearFactory::enable(const Enable en, const bool clear_fault){
    if(en == EN){
        const auto extid = CgId::from_parts(cybergear::Command::EN_MOT, host_id, node_id).to_extid();
        return hal::ClassicCanFrame::from_empty_data(extid);
    }else{
        uint64_t data_u64 = 0;
        data_u64 |= (clear_fault) ? 1u << 0 : 0;

        // 正常运行时，data区需清0；
        // byte[0]=1 时：清故障；
        return hal::ClassicCanFrame::from_parts(
            CgId::from_parts(cybergear::Command::DISEN_MOT, host_id, node_id).to_extid(), 
            hal::ClassicCanPayload::from_u64(data_u64)
        );
    }
}


hal::ClassicCanFrame CyberGearFactory::set_current_as_machine_home(){
    static constexpr size_t LENGTH = 8;
    static constexpr std::array<uint8_t, LENGTH> buffer = {
        1,
        0, 0, 0, 
        0, 0, 0, 0
    };

    const auto extid = CgId::from_parts(cybergear::Command::SET_MACHINE_HOME, 
        host_id, node_id).to_extid();

    return hal::ClassicCanFrame::from_parts(
        extid, 
        hal::ClassicCanPayload::from_u8x8(buffer)
    );
}

// hal::ClassicCanFrame CyberGearFactory::change_node_id(const uint8_t node_id){
//     node_id = node_id;
//     return hal::ClassicCanFrame(
//         CgId::from_parts(cybergear::Command::SET_CAN_ID, host_id, node_id).to_bits(),
//             0, 0);
// }

hal::ClassicCanFrame CyberGearFactory::request_read_para(const uint16_t idx){
    static constexpr size_t LENGTH = 8;
    std::array<uint8_t, LENGTH> buffer = {
        static_cast<uint8_t>(idx & 0xFF),
        static_cast<uint8_t>(idx >> 8),
        0, 0, 
        0, 0, 0, 0
    };

    return hal::ClassicCanFrame::from_parts(
        CgId::from_parts(cybergear::Command::READ_PARA, host_id, node_id).to_extid(), 
        hal::ClassicCanPayload::from_u8x8(std::move(buffer))
    );
}

hal::ClassicCanFrame CyberGearFactory::request_write_para(
    const uint16_t idx, 
    const uint32_t param_bits
){

    static constexpr size_t LENGTH = 8;
    std::array<uint8_t, LENGTH> buffer = {
        static_cast<uint8_t>(idx & 0xFF),
        static_cast<uint8_t>(idx >> 8),
        0, 0, 
        static_cast<uint8_t>(param_bits & 0xff),
        static_cast<uint8_t>((param_bits >> 8) & 0xff),
        static_cast<uint8_t>((param_bits >> 16) & 0xff),
        static_cast<uint8_t>((param_bits >> 24) & 0xff)
    };


    return hal::ClassicCanFrame::from_parts(
        CgId::from_parts(cybergear::Command::WRITE_PARA, host_id, node_id).to_extid(), 
        hal::ClassicCanPayload::from_u8x8(std::move(buffer))
    );
}



#if 1
IResult<> CyberGearRx::on_receive(const ClassicCanFrame & frame){
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

IResult<> CyberGearRx::on_mcu_id_feed_back(
    const uint32_t id_u32, 
    const uint64_t payload_u64, 
    const uint8_t dlc
){
    (void)id_u32;

    if (dlc != 8){
        return Err(Error::RET_DLC_SHORTER);
    }

    device_mcu_id_ = Some(payload_u64);
    return Ok();
}



IResult<> CyberGearRx::on_ctrl2_feed_back(
    const uint32_t id_u32, 
    const uint64_t payload_u64, 
    const uint8_t dlc
){
    (void)id_u32;

    if(dlc != sizeof(RxContext)){
        return Err(Error::RET_DLC_SHORTER);
    }

    const RxContext context = {payload_u64};

    feedback_.radians =         context.radians().get().to<iq16>();
    feedback_.omega =       context.omega().get().to<iq16>();
    feedback_.torque =      context.torque().get().to<iq16>();
    feedback_.celsius = context.temperature().get().to_celsius();

    return Ok();
}
#endif

}
