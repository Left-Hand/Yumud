#include "ads1115.hpp"
#include "core/debug/debug.hpp"
#include "primitive/analog_channel.hpp"

// #define ADS1115_DEBUG

#ifdef ADS1115_DEBUG
#undef ADS1115_DEBUG
#define ADS1115_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define ADS1115_PANIC(...) PANIC{__VA_ARGS__}
#define ADS1115_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define ADS1115_DEBUG(...)
#define ADS1115_PANIC(...)  PANIC_NSRC()
#define ADS1115_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif


namespace ymd::drivers{

using Error = ADS1115::Error;

template<typename T = void>
using IResult = Result<T, Error>;  

IResult<> ADS1115::read_reg(const RegAddr addr, uint16_t & data){
    if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, std::endian::little);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> ADS1115::write_reg(const RegAddr addr, const uint16_t data){
    if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data, std::endian::little);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> ADS1115::validate(){
    if(const auto res = i2c_drv_.validate();
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> ADS1115::start_conv(){
    auto reg = RegCopy(config_reg);
    reg.busy = true;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    reg.busy = false;
    reg.apply();
    return Ok();
}


IResult<> ADS1115::set_threshold(int16_t low, int16_t high){
    auto low_thresh_reg_copy = RegCopy(low_thresh_reg);
    auto high_thresh_reg_copy = RegCopy(high_thresh_reg);
    low_thresh_reg_copy.data = low;
    high_thresh_reg_copy.data = high;

    if(const auto res = write_reg(low_thresh_reg_copy);
        res.is_err()) return res;
    if(const auto res = write_reg(high_thresh_reg_copy);
        res.is_err()) return res;
    return Ok();
}

IResult<> ADS1115::enable_cont_mode(Enable en){
    auto reg = RegCopy(config_reg);
    reg.oneshot_en = (en == DISEN);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}

IResult<> ADS1115::set_pga(const PGA pga){
    auto reg = RegCopy(config_reg);
    reg.pga = uint16_t(pga);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}

IResult<> ADS1115::set_mux(const MUX mux){
    auto reg = RegCopy(config_reg);
    reg.mux = uint16_t(mux);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}

IResult<> ADS1115::set_data_rate(const DataRate data_rate){
    auto reg = RegCopy(config_reg);
    reg.data_rate = uint16_t(data_rate);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}

IResult<bool> ADS1115::is_busy(){
    auto & reg = config_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.busy));
}


Option<real_t> ADS1115::get_voltage(){
    auto & reg = conversion_reg;
    if(read_reg(reg.ADDRESS, reg.as_mut_bits()).is_err()) return None;
    return Some(iq16::from_bits(~reg.data) * 3.3_r);
    // return None;
}

}