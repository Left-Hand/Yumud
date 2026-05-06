#include "cybergear_runtime.hpp"

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
using namespace ymd::robots::cybergear;

using ClassicCanFrame = hal::ClassicCanFrame;
using TemperatureCode = cybergear::TemperatureCode;
using RadCode = cybergear::RadCode;
using OmegaCode = cybergear::OmegaCode;
using TorqueCode = cybergear::TorqueCode;
using KpCode = cybergear::KpCode;
using KdCode = cybergear::KdCode;

using Error = cybergear::Error;

template<typename T = void>
using IResult = Result<T, Error>;


namespace ymd::robots::cybergear{



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


}
