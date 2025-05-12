#include "AK09911C.hpp"


#define DEBUG_EN

#ifdef DEBUG_EN

#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif


using namespace ymd;
using namespace ymd::drivers;

using Error = AK09911C::Error;
template<typename T = void>
using IResult = Result<T, Error>;

IResult<> AK09911C::init(){
    if(const auto res = validate(); res.is_err()) return res;

    return Ok();
}

IResult<> AK09911C::update(){
    return Ok();
}


IResult<Vector3_t<int8_t>> AK09911C::get_coeff(){
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

IResult<> AK09911C::validate(){
    const auto res = 
        read_reg(wia1_reg) | read_reg(wia2_reg);

    if(res.is_err()) return CHECK_RES(res, "failed to read reg when validate");
    if(wia1_reg.as_val() != wia1_reg.KEY) return CHECK_ERR(Err(Error::WrongCompanyId),  
        "wrong company id, correct is", wia1_reg.KEY, "but read is", wia1_reg.as_val());
    if(wia2_reg.as_val() != wia2_reg.KEY) return CHECK_ERR(Err(Error::WrongDeviceId), 
        "wrong device id, correct is", wia2_reg.KEY, "but read is", wia2_reg.as_val());

    return Ok();
}

IResult<bool> AK09911C::is_data_ready(){
    if(const auto res = read_reg(st1_reg); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok(st1_reg.is_data_ready());
}

IResult<bool> AK09911C::is_data_overrun(){
    if(const auto res = read_reg(st1_reg); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok(st1_reg.is_data_overrun());
}

IResult<> AK09911C::enable_hs_i2c(const Enable en){
    st1_reg.hsm = (en == EN) ? 1 : 0;
    return write_reg(st1_reg);
}

IResult<> AK09911C::set_mode(const Mode mode){
    cntl2_reg.mode = mode;
    return write_reg(cntl2_reg);
}

IResult<> AK09911C::reset(){
    cntl3_reg.srst = 1;
    const auto res = write_reg(cntl3_reg);
    cntl3_reg.srst = 0;
    return res;
}

IResult<> AK09911C::set_odr(const Odr odr){
    return set_mode(static_cast<Mode>(odr));
}