#include "zdt_stepper_phy.hpp"

using namespace ymd;
using namespace ymd::robots::zdtmotor;
using namespace ymd::robots::zdtmotor::prelude;


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
    buf.append_unchecked(id.to_u8());
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
    if(may_uart_.is_some()){
        uart_write_bytes(
            may_uart_.unwrap(), 
            id, func_code, bytes
        );
    }else if(may_can_.is_some()){
        can_write_bytes(
            may_can_.unwrap(), 
            id, func_code, bytes
        );
    }else{
        PANIC{"no phy available"};
    }
}