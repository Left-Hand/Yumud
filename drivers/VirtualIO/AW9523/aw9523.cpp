#include "aw9523.hpp"
#include "core/clock/clock.hpp"


using namespace ymd;
using namespace ymd::drivers;

using Error = AW9523::Error;

template<typename T = void>
using IResult = Result<T, Error>;

#define GUARD_INDEX(index)\
if(not is_index_valid(index))\
    return Err(Error::IndexOutOfRange);\

IResult<> AW9523::init(){
    if(const auto res = validate();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = reset();
        res.is_err()) return res;
    clock::delay(2ms);
    if(const auto res = set_led_current_limit(CurrentLimit::Low);
        res.is_err()) return res;
    for(uint8_t i = 0; i < 16; i++){
        if(const auto res = set_led_current(std::bit_cast<hal::PinSource>(
            hal::PinMask::from_index(i).as_u16()), 
            0); res.is_err()) return Err(res.unwrap_err());
    }
    led_mode_ = hal::PinMask(0xffff);
    return Ok();
}


IResult<> AW9523::write_by_index(const size_t index, const BoolLevel data){
    GUARD_INDEX(index);
    buf_mask_ = buf_mask_.modify(index, data);
    return write_by_mask(hal::PinMask(buf_mask_));
}

IResult<BoolLevel> AW9523::read_by_index(const size_t index){
    GUARD_INDEX(index);
    if(const auto res = read_mask();
        res.is_err()) return Err(res.unwrap_err());
    return Ok(BoolLevel::from(buf_mask_.test(index)));
}

IResult<> AW9523::set_mode(const size_t index, const hal::GpioMode mode){
    GUARD_INDEX(index);
    uint16_t mask = 1 << index;
    if(mode.is_in_mode()) 
        dir_reg |= mask;
    else 
        dir_reg &= ~mask;
    if(const auto res = write_reg(RegAddress::Dir, dir_reg);
        res.is_err()) return Err(res.unwrap_err());

    if(index < 8){
        ctl.p0mod = mode.is_outpp_mode();
        if(const auto res = write_reg(RegAddress::Ctl, ctl);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

IResult<> AW9523::enable_irq_by_index(const size_t index, const bool en ){
    GUARD_INDEX(index);
    return write_reg(RegAddress::Inten, (uint8_t)(en << index));
}

IResult<> AW9523::enable_led_mode(const hal::PinSource pin, const bool en){
    uint index = CTZ((uint16_t)pin);
    GUARD_INDEX(index);
    led_mode_ = led_mode_.modify(index, BoolLevel::from(en));
    return write_reg(RegAddress::LedMode, led_mode_.as_u16());
}

IResult<> AW9523::set_led_current_limit(const CurrentLimit limit){
    ctl.isel = (uint8_t)limit;
    return write_reg(RegAddress::Ctl, ctl);
}

IResult<> AW9523::set_led_current(const hal::PinSource pin, const uint8_t current){
    uint index = CTZ((uint16_t)pin);
    GUARD_INDEX(index);
    return write_reg(get_dim_addr(index), current);
}


IResult<> AW9523::validate(){
    uint8_t chip_id;
    if(const auto res = read_reg(RegAddress::ChipId, chip_id);
        res.is_err()) return res;
    if(chip_id != VALID_CHIP_ID)
        return Err(Error::WrongChipId);
    return Ok();
}
