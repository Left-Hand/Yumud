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
    led_mode_reg.mask = hal::PinMask(0xffff);
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

    {
        auto reg = RegCopy(dir_reg);
        if(mode.is_in_mode()) 
            reg.mask = reg.mask.modify(index, HIGH);
        else 
            reg.mask = reg.mask.modify(index, LOW);
        
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    if(index < 8){
        auto reg = RegCopy(ctl_reg);
        reg.p0mod = mode.is_outpp_mode();
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

IResult<> AW9523::enable_irq_by_index(const size_t index, const Enable en ){
    GUARD_INDEX(index);
    auto reg = RegCopy(inten_reg);
    reg.mask = reg.mask.modify(index, BoolLevel::from(en == EN));
    return write_reg(reg);
}

IResult<> AW9523::enable_led_mode(const hal::PinSource pin, const Enable en){
    uint index = CTZ((uint16_t)pin);
    GUARD_INDEX(index);
    auto reg = RegCopy(led_mode_reg);
    reg.mask = reg.mask.modify(index, BoolLevel::from(en == EN));
    return write_reg(reg);
}

IResult<> AW9523::set_led_current_limit(const CurrentLimit limit){
    auto reg = RegCopy(ctl_reg);
    reg.isel = (uint8_t)limit;
    return write_reg(reg);
}

IResult<> AW9523::set_led_current(const hal::PinSource pin, const uint8_t current){
    uint index = CTZ(std::bit_cast<uint16_t>(pin));
    GUARD_INDEX(index);
    return write_reg(get_dim_addr(index), current);
}


IResult<> AW9523::validate(){
    if(const auto res = read_reg(chip_id_reg);
        res.is_err()) return res;
    if(chip_id_reg.id != VALID_CHIP_ID)
        return Err(Error::WrongChipId);
    return Ok();
}
