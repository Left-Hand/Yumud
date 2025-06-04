#include "ads112c04.hpp"



#ifdef ADS112C04_DEBUG
#define ADS112C04_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define ADS112C04_DEBUG(...)
#endif



using namespace ymd;
using namespace ymd::drivers;


using Error = ADS112C04::Error;
template<typename T = void>
using IResult = Result<T, Error>;

IResult<> ADS112C04::init(){
    config3_reg.__resv__ = 0;

    if(const auto res = validate(); 
        res.is_err()) return res;
    return Ok();
}

IResult<> ADS112C04::validate(){
    TODO();
    return Ok();
}


IResult<> ADS112C04::set_mux(const Mux mux){
    auto reg = RegCopy(config0_reg);
    reg.mux = mux;
    return write_reg(reg);
}

IResult<> ADS112C04::set_gain(const Gain gain){
    auto reg = RegCopy(config0_reg);
    reg.gain = gain;
    return write_reg(reg);
}

IResult<> ADS112C04::enable_turbo(const Enable en){
    auto reg = RegCopy(config1_reg);
    reg.turbo_mode = en == EN;
    return write_reg(reg);
}

IResult<bool> ADS112C04::is_done(){
    if(const auto res = read_reg(config2_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(config2_reg.conv_done));
}

IResult<> ADS112C04::set_idac(const IDAC idac){
    auto reg = RegCopy(config2_reg);
    reg.idac = idac;
    return write_reg(reg);
}

IResult<> ADS112C04::set_data_rate(const DataRate data_rate){
    auto reg = RegCopy(config1_reg);
    reg.data_rate = data_rate;
    return write_reg(reg);
}