#include "TM1637.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = TM1637_Phy::Error;


Result<void, Error> TM1637_Phy::write_sram(const std::span<const uint8_t> pbuf, const PulseWidth pw){
    if(pbuf.size() > TM1637::CGRAM_MAX_LEN) return Err(Error::DisplayLengthTooLong);

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

    // const auto guard = i2c_.create_guard();

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

Result<uint8_t, Error> TM1637_Phy::read_key(){
    const auto command1 = DataCommand{
        .write_else_read = false,//write
        .addr_inc_en = false
    }.as_u8();

    i2c_.begin(hal::LockRequest(command1, 0));

    uint32_t data;
    i2c_.read(data, ACK);
    i2c_.end();
    return Ok(data);
}

Result<TM1637::KeyEvent, Error> TM1637::read_key(){
    const auto res = phy_.read_key();
    if(res.is_err()) return Err(res.unwrap_err());
    return KeyEvent::from_u8(res.unwrap());
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

Result<void, Error> TM1637::flush(){
    //TODO 更换为数据利用率更高的更新算法

    const bool changed = buf_.changed();
    if(changed){
        const auto res = phy_.write_screen(buf_.to_span());
        buf_.flush();
        return res;
    }
    return Ok();
}
