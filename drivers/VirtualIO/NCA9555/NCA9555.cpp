#include "NCA9555.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Error = NCA9555::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> NCA9555::init(){
    return set_inversion(hal::PinMask(0));
}


IResult<> NCA9555::set_inversion(const hal::PinMask mask){
    auto reg = RegCopy(inversion_reg);
    if(mask == reg.mask) return Ok();
    reg.mask = mask;
    return write_reg(reg);
}


IResult<> NCA9555::write_port(const uint16_t _mask){
    const auto mask = hal::PinMask(_mask);
    auto reg = RegCopy(output_reg);
    if(mask == reg.mask) return Ok();
    reg.mask = mask;
    return write_reg(reg);
}

IResult<uint16_t> NCA9555::read_port(){
    auto & reg = input_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.mask.as_u16());
}


IResult<> NCA9555::set_mode(const size_t index, const hal::GpioMode mode){
    if(index > 15) return Err(Error::IndexOutOfRange);

    auto reg = RegCopy(config_reg);
    const auto new_mask = reg.mask.modify(
        index, 
        BoolLevel::from(mode.is_in_mode()));

    if(reg.mask != new_mask){
        reg.mask = new_mask;
        return write_reg(reg);
    }
    return Ok();
}