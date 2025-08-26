#include "zdt_stepper.hpp"

using namespace ymd;
using namespace ymd::robots::zdtmotor;
using namespace ymd::robots::zdtmotor::prelude;

IResult<> ZdtStepper::set_position(ZdtStepper::PositionSetpoint targ){
    return write_payload(payloads::SetPosition{
        .is_ccw = (targ.position.is_negative()),
        .rpm = Rpm::from_speed(targ.speed),
        .acc_level = AcclerationLevel::from_u8(0),
        .pulse_cnt = PulseCnt::from_position(targ.position.abs()),
        .is_absolute = true,
        .is_sync = is_sync_
    });
}

IResult<> ZdtStepper::set_speed(ZdtStepper::SpeedSetpoint targ){
    return write_payload(payloads::SetSpeed{
        .is_ccw = targ.speed < 0,
        .rpm = Rpm::from_speed(ABS(targ.speed)),
        .acc_level = AcclerationLevel::from(0),
        .is_absolute = true,
        .is_sync = is_sync_
    });
}

IResult<> ZdtStepper::brake(){
    return write_payload(payloads::Brake{
        .is_sync = is_sync_
    });
}

IResult<> ZdtStepper::set_subdivides(const uint16_t subdivides){
    if(subdivides > 256) 
        return Err(Error::SubDivideOverflow);
    return write_payload(payloads::SetSubDivides{
        .subdivides = uint8_t(subdivides & 0xff)
    });
}

IResult<> ZdtStepper::activate(const Enable en){
    return write_payload(payloads::Actvation{
        .en = en == EN,
        .is_sync = is_sync_
    });
}


IResult<> ZdtStepper::trig_cali(){
    return write_payload(payloads::TrigCali::from_default());  
}

IResult<> ZdtStepper::query_homming_paraments(){
    return write_payload(payloads::QueryHommingParaments{});
}

IResult<> ZdtStepper::trig_homming(const HommingMode mode){
    return write_payload(payloads::TrigHomming{
        .homming_mode = mode,
        .is_sync = is_sync_
    });
}

void ZdtMotorPhy::can_write_bytes(
    hal::Can & can, 
    const NodeId id, 
    const FuncCode func_code,
    const std::span<const uint8_t> bytes
){
    auto iter = Bytes2CanMsgIterator(id, func_code, bytes);
    while(iter.has_next()){
        const auto msg = iter.next();
        can.write(msg).examine();
    }
}

void ZdtMotorPhy::uart_write_bytes(
    hal::Uart & uart, 
    const NodeId id, 
    const FuncCode func_code,
    const std::span<const uint8_t> bytes
){
    Buf buf;
    buf.append_unchecked(id.as_u8());
    buf.append_unchecked(std::bit_cast<uint8_t>(func_code));
    buf.append_unchecked(bytes);

    uart.writeN(
        reinterpret_cast<const char *>(buf.data()),
        buf.size()
    );
}

void ZdtMotorPhy::write_bytes(
    const NodeId id, 
    const FuncCode func_code,
    const std::span<const uint8_t> bytes
){
    // DEBUG_PRINTLN(std::hex, "tx", bytes);
    if(uart_.is_some()){
        uart_write_bytes(
            uart_.unwrap(), 
            id, func_code, bytes
        );
    }else if(can_.is_some()){
        can_write_bytes(
            can_.unwrap(), 
            id, func_code, bytes
        );
    }else{
        __builtin_unreachable();
    }
}