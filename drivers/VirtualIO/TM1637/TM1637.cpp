#include "TM1637.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = TM1637_Phy::Error;


Result<void, Error> TM1637_Phy::write_sram(const std::span<const uint8_t> pbuf, const PulseWidth pw){
    const auto guard = i2c_.create_guard();

    const auto command1 = DataCommand{
        .write_else_read = true,//write
        .addr_inc_en = true
    }.as_u8();

    const auto command2 = AddressCommand{
        .addr = CGRAM_BEGIN_ADDR
    }.as_u8();

    const auto command3 = DisplayCommand{
        .pulse_width = pw,
        .display_en = true
    }.as_u8();

    i2c_.begin(hal::LockRequest(command1, 0));
    i2c_.end();
    i2c_.begin(hal::LockRequest(command2, 0));
    for(size_t i = 0; i < pbuf.size(); i++){
        i2c_.write(pbuf[i]);
    }
    i2c_.end();
    i2c_.begin(hal::LockRequest(command3, 0));
    i2c_.end();
    return Ok();
}


Result<void, Error> TM1637_Phy::write_reg(const uint8_t addr, const uint8_t data){
    TODO();

    return Ok();
}

Result<void, Error> TM1637_Phy::write_burst(const uint8_t addr, const std::span<const uint8_t> pbuf){
    TODO();

    return Ok();
}

Result<void, Error> TM1637_Phy::write_screen(const std::span<const uint8_t, 6> pbuf){
    TODO();

    return Ok();
}

// Result<TM1637_Phy::KeyEvent, Error> TM1637_Phy::read_key(){
//     TODO();

//     return Ok();
// }