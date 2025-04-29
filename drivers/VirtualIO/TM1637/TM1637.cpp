#include "TM1637.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = TM1637_Phy::Error;

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