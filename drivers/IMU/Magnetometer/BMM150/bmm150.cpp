#include "bmm150.hpp"


using namespace ymd;
using namespace ymd::drivers;

using Self = BMM150;

using Error = Self::Error;  

template<typename T = void>
using IResult = Result<T, Error>;

[[nodiscard]] IResult<> Self::init(const Config & cfg){
    if(const auto res = validate();
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

[[nodiscard]] IResult<> Self::update(){
    return Ok();
}

[[nodiscard]] IResult<> Self::validate(){
    auto & reg = regs_.chipid_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    if(reg.chipid != reg.KEY);
    return Err(Error::UnknownDevice);
}

[[nodiscard]] IResult<> Self::reset(){
    auto reg = RegCopy(regs_.power_control_reg);
    reg.soft_reset = 1;
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    reg.soft_reset = 0;
    reg.apply();
    return Ok();
}

[[nodiscard]] IResult<math::Vec3<iq24>> Self::read_mag(){
    TODO();
    return Ok(math::Vec3<iq24>{0,0,0});
}