#include "AK09911C.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = AK09911C::Error;
template<typename T = void>
using IResult = Result<T, Error>;

IResult<Vector3_t<uint8_t>> get_coeff(){
    // Vector3_t coeff = {};
    if(const auto res = write_reg(0x0a, 0x0f);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(asax_reg.address, asax_reg.as_ref());
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(asay_reg.address, asay_reg.as_ref());
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(asaz_reg.address, asaz_reg.as_ref());
        res.is_err()) return Err(res.unwrap_err());
    return Ok(Vector3_t<uint8_t>{asax_reg.data, asay_reg.data, asaz_reg.data});
}