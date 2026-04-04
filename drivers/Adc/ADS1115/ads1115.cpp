#include "ads1115.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"


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


using namespace ymd;
using namespace ymd::drivers;

using Error = ADS1115::Error;

template<typename T = void>
using IResult = Result<T, Error>;  


IResult<> ADS1115::validate(){
    if(const auto res = i2c_drv_.validate();
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> ADS1115::start_conv(){
    auto reg = RegCopy(regs_.config_reg);

    auto guard = make_scope_guard([&]{
        reg.busy = false;
        reg.apply();
    });


    reg.busy = true;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}


IResult<> ADS1115::set_threshold(int16_t low, int16_t high){
    auto low_thresh_reg_copy = RegCopy(regs_.low_thresh_reg);
    auto high_thresh_reg_copy = RegCopy(regs_.high_thresh_reg);
    low_thresh_reg_copy.bits = low;
    high_thresh_reg_copy.bits = high;

    if(const auto res = write_reg(low_thresh_reg_copy);
        res.is_err()) return res;
    if(const auto res = write_reg(high_thresh_reg_copy);
        res.is_err()) return res;
    return Ok();
}

IResult<> ADS1115::enable_cont_mode(Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.oneshot_en = (en == DISEN);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}

IResult<> ADS1115::set_pga(const PGA pga){
    auto reg = RegCopy(regs_.config_reg);
    reg.pga = uint16_t(pga);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}

IResult<> ADS1115::set_mux(const MUX mux){
    auto reg = RegCopy(regs_.config_reg);
    reg.mux = uint16_t(mux);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}

IResult<> ADS1115::set_datarate(const DataRate datarate){
    auto reg = RegCopy(regs_.config_reg);
    reg.datarate = uint16_t(datarate);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    return Ok();
}

IResult<bool> ADS1115::is_busy(){
    auto & reg = regs_.config_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.busy));
}


Option<int16_t> ADS1115::get_conversion_result(){
    auto & reg = regs_.conversion_reg;
    if(read_reg(reg.REG_ADDR, reg.as_bits_mut()).is_err()) return None;
    return Some(reg.bits);
}


IResult<> ADS1115::read_reg(const uint8_t reg_addr, uint16_t & reg_val){
    if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val, std::endian::little);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> ADS1115::write_reg(const uint8_t reg_addr, const uint16_t reg_val){
    if(const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val, std::endian::little);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

