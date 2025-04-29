#include "TM1637.hpp"
#include "core/debug/debug.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = TM1637_Phy::Error;

Result<void, Error> TM1637_Phy::write_byte(const uint8_t data){
    sda_gpio_.outod();
    for(uint8_t mask = 0x01; mask; mask <<= 1){
        scl_gpio_.clr();
        sda_gpio_.write(BoolLevel::from(mask & data));
        udelay(3);
        scl_gpio_.set();
        udelay(3);
    }

    return wait_ack();
}


Result<void, Error> TM1637_Phy::wait_ack(){
    sda_gpio_.inpu();
    scl_gpio_.clr();
    udelay(5);
    scl_gpio_.set();

    bool ovt = false;
    const auto m = micros();
    while(sda_gpio_.read() == HIGH){
        if(micros() - m >= 100){
            ovt = true;
            break;
        }
        udelay(1);
    }

    scl_gpio_.set();
    udelay(2);
    scl_gpio_.clr();
    
    if(ovt){
        // iic_stop();
        return Err(hal::HalResult::AckTimeout);
    }else{
        return Ok();
    }
}

// Result<void, Error> TM1637_Phy::read_key(uint8_t & data){
//     uint8_t ret = 0;

//     for(uint8_t i = 0; i < 8; i++){
//         scl_gpio_.clr();
//         ret = ret >> 1;
//         udelay(30);
//         scl_gpio_.set();
//         ret = ret | ((sda_gpio_.read() == HIGH) ? 0x80 : 0x00);
//         udelay(30);
//     }

//     data = ret;
//     return Ok();
// }

Result<void, Error> TM1637_Phy::iic_start(const uint8_t data){
    scl_gpio_.outod(HIGH);
    sda_gpio_.outod(HIGH);
    udelay(2);
    sda_gpio_.clr();
    return write_byte(data);
}

Result<void, Error> TM1637_Phy::iic_stop(){
    scl_gpio_.clr();
    sda_gpio_.outod();
    udelay(2);
    sda_gpio_.clr();
    udelay(2);
    scl_gpio_.set();
    udelay(2);
    sda_gpio_.set();

    return Ok();
}

Result<void, Error> TM1637_Phy::write_sram(const std::span<const uint8_t> pbuf, const PulseWidth pw){
    if(pbuf.size() > TM1637::CGRAM_MAX_LEN) return Err(Error::DisplayLengthTooLong);

    const auto command1 = DataCommand{
        .read_key = false,//write
        .addr_inc_disen = false
    }.as_u8();

    const auto command2 = AddressCommand{
        .addr = CGRAM_BEGIN_ADDR
    }.as_u8();

    const auto command3 = DisplayCommand{
        .pulse_width = pw,
        .display_en = true
    }.as_u8();

    // const auto guard = i2c_.create_guard();

    iic_start(command1);
    iic_stop();

    delay(1);

    iic_start(command2);
    for(size_t i = 0; i < pbuf.size(); i++){
        write_byte(pbuf[i]);
    }
    iic_stop();

    delay(1);

    iic_start(command3);
    iic_stop();

    return Ok();
}

Result<uint8_t, Error> TM1637_Phy::read_key(){
    const auto command1 = DataCommand{
        .read_key = true,//write
        .addr_inc_disen = true
    }.as_u8();

    // i2c_.begin(hal::LockRequest(command1, 0));

    // uint32_t data;
    // i2c_.read(data, ACK);
    // i2c_.end();
    // return Ok(data);
    return Ok(0);
}

Result<TM1637::KeyEvent, Error> TM1637::read_key(){
    const auto res = phy_.read_key();
    if(res.is_err()) return Err(res.unwrap_err());
    return KeyEvent::from_u8(res.unwrap());
}

Result<void, Error> TM1637::flush(){
    //TODO 更换为数据利用率更高的更新算法

    const bool changed = buf_.changed();
    if(changed){
        // const auto res = phy_.write_screen(buf_.to_span());
        const auto res = phy_.write_sram(buf_.to_span(), PulseWidth::_1_16);
        buf_.flush();
        return res;
    }
    return Ok();
}
