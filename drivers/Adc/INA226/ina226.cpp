#include "ina226.hpp"

#include "core/debug/debug.hpp"

#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


#define INA226_DEBUG_EN 1
// #define INA226_DEBUG_EN 0

#if INA226_DEBUG_EN
#define INA226_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA226_PANIC(...) PANIC(__VA_ARGS__)
#define INA226_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


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
#define INA226_DEBUG(...)
#define INA226_PANIC(...)  PANIC_NSRC()
#define INA226_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif

using Error = INA226::Error;

template<typename T = void>
using IResult = Result<T, Error>;


static constexpr INA226::AverageTimes times_to_avtimes(const uint16_t times){
    const uint8_t temp = CTZ(times);

    if(times <= 64){
        return std::bit_cast<INA226::AverageTimes>(uint16_t(temp / 2));
    }else{
        return std::bit_cast<INA226::AverageTimes>(uint16_t(4 + (temp - 7))); 
    }
} 

IResult<> INA226::update(){
    if(const auto res = this->read_reg(bus_volt_reg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(current_reg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(power_reg);
        res.is_err()) return res;

    return Ok();
}


IResult<> INA226::write_reg(const RegAddr addr, const uint16_t data){
    if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA226::read_reg(const RegAddr addr, uint16_t & data){
    if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA226::read_reg(const RegAddr addr, int16_t & data){
    if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}
IResult<> INA226::init(const Config & cfg){
    config_reg.rst = 0b0;
    config_reg.__resv__ = 0b100;

    INA226_DEBUG("init");
    
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = reset();
        res.is_err()) return res;
    if(const auto res = set_average_times(cfg.average_times);
        res.is_err()) return res;
    if(const auto res = set_bus_conversion_time(cfg.bus_conv_time);
        res.is_err()) return res;
    if(const auto res = set_shunt_conversion_time(cfg.shunt_conv_time);
        res.is_err()) return res;
    if(const auto res = enable_bus_voltage_measure(EN);
        res.is_err()) return res;
    if(const auto res = enable_continuous_measure(EN);
        res.is_err()) return res;
    if(const auto res = enable_shunt_voltage_measure(EN);
        res.is_err()) return res;
    if(const auto res = set_scale(cfg.sample_res_mohms, cfg.max_current_a);
        res.is_err()) return res;
    return Ok();
}

IResult<> INA226::set_scale(const uint32_t sample_res_mohms, const uint32_t max_current_a){
    INA226_DEBUG(sample_res_mohms, max_current_a);
    
    current_lsb_ma_ = iq16(int(max_current_a) * 1000) >> 15;
    // INA226_DEBUG(current_lsb_ma_, sample_res_mohms * max_current_a);
    const auto val = int(0.00512 * 32768 * 1000) / (sample_res_mohms * max_current_a);
    // PANIC(calibration_reg.as_bits());
    auto reg = RegCopy(calibration_reg);
    reg.as_mut_bits() = int16_t(val);
    // PANIC(reg.as_bits(), val);
    return write_reg(reg);
}


IResult<> INA226::set_average_times(const uint16_t times){
    return set_average_times(times_to_avtimes(times));
}

IResult<iq16> INA226::get_voltage(){
    return Ok(bus_volt_reg.as_bits() * VOLTAGE_LSB_MV / 1000);
    // return bus_voltage_reg.as_bits();
}

IResult<int> INA226::get_shunt_voltage_uv(){
    const auto val = shunt_volt_reg.as_bits();
    //val * 2.5
    return Ok((val << 1) + (val >> 1));
}

IResult<iq16> INA226::get_shunt_voltage(){
    const int uv = ({
        const auto res = get_shunt_voltage_uv();
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    return Ok(iq16(uv / 100) / 10000);
}

IResult<iq16> INA226::get_current(){
    return Ok(current_reg.as_bits() * current_lsb_ma_ / 1000);
    // return current_reg.as_bits() ;
}

IResult<iq16> INA226::get_power(){
    return Ok(power_reg.as_bits() * current_lsb_ma_ / 40);
    // return power_reg.as_bits();
}

IResult<> INA226::set_average_times(const AverageTimes times){
    auto reg = RegCopy(config_reg);
    reg.average_times = times;
    return write_reg(reg);
}

IResult<> INA226::set_bus_conversion_time(const ConversionTime time){
    auto reg = RegCopy(config_reg);
    reg.bus_voltage_conversion_time = time;
    return write_reg(reg);
}

IResult<> INA226::set_shunt_conversion_time(const ConversionTime time){
    auto reg = RegCopy(config_reg);
    reg.shunt_voltage_conversion_time = time;
    return write_reg(reg);
}

IResult<> INA226::reset(){
    auto reg = RegCopy(config_reg);
    reg.rst = 1;
    return write_reg(reg);
}

IResult<> INA226::enable_shunt_voltage_measure(const Enable en){
    auto reg = RegCopy(config_reg);
    reg.shunt_voltage_enable = en == EN;
    return write_reg(reg);
}

IResult<> INA226::enable_bus_voltage_measure(const Enable en){
    auto reg = RegCopy(config_reg);
    reg.bus_voltage_enable = en == EN;
    return write_reg(reg);
}

IResult<> INA226::enable_continuous_measure(const Enable en){
    auto reg = RegCopy(config_reg);
    reg.continuos = en == EN;
    return write_reg(reg);
}

IResult<> INA226::enable_alert_latch(const Enable en){
    auto  reg = RegCopy(mask_reg);
    reg.alert_latch_enable = en == EN;
    return write_reg(reg);
}

IResult<> INA226::validate(){
    if(const auto res = i2c_drv_.validate(); res.is_err()){
        return CHECK_ERR(Err(res.unwrap_err()), "INA226 i2c lost");
    }

    if(const auto res = this->read_reg(chip_id_reg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(manufacture_reg);
        res.is_err()) return res;

    if((chip_id_reg.as_bits() != VALID_CHIP_ID)) 
        return CHECK_ERR(Err(Error::ChipIdVerifyFailed));
    if((manufacture_reg.as_bits() != VALID_MANU_ID)) 
        return CHECK_ERR(Err(Error::ManuIdVerifyFailed));

    return Ok();
}