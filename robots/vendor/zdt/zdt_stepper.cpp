#include "zdt_stepper.hpp"

using namespace ymd;
using namespace ymd::robots;

void ZdtStepper::set_target_position(const real_t pos){
    write_payload(Payloads::SetPosition{
        .is_ccw = pos < 0,
        .rpm = Rpm::from(0.07_r),
        .acc_level = AcclerationLevel::from(0),
        .pulse_cnt = PulseCnt::from(ABS(pos)),
        .is_absolute = false,
        .is_sync = is_sync_
    });
}

void ZdtStepper::set_target_speed(const real_t spd){
    write_payload(Payloads::SetSpeed{
        .is_ccw = spd < 0,
        .rpm = Rpm::from(ABS(spd)),
        .acc_level = AcclerationLevel::from(0),
        .is_absolute = false,
        .is_sync = is_sync_
    });     
}

void ZdtStepper::brake(){
    write_payload(Payloads::Brake{
        .is_sync = is_sync_
    });
}

void ZdtStepper::set_subdivides(const uint16_t subdivides){
    if(subdivides > 256) PANIC();
    write_payload(Payloads::SetSubDivides{
        .subdivides = uint8_t(subdivides & 0xff)
    });
}

void ZdtStepper::enable(const Enable en){
    write_payload(Payloads::Actvation{
        .en = en == EN,
        .is_sync = is_sync_
    });
}


void ZdtStepper::trigger_cali(){
    write_payload(Payloads::TrigCali::from_default());  
}

void ZdtStepper::query_homming_paraments(){
    write_payload(Payloads::QueryHommingParaments{});
}

void ZdtStepper::trig_homming(const HommingMode mode){
    write_payload(Payloads::TrigHomming{
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

    while(true){
        const auto may_msg = iter.next();
        if(may_msg.is_none()) break;
        can.write(may_msg.unwrap());
    }
}

void ZdtMotorPhy::uart_write_bytes(
    hal::Uart & uart, 
    const NodeId id, 
    const FuncCode func_code,
    const std::span<const uint8_t> bytes
){
    uart.write1(id.to_u8());
    uart.write1(std::bit_cast<uint8_t>(func_code));
    uart.writeN(reinterpret_cast<const char *>(
        bytes.data()), bytes.size());

    DEBUG_PRINTLN(id.to_u8(), std::bit_cast<uint8_t>(func_code), bytes);
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
        PANIC();
    }
}