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


template<typename T = void>
using IResult = INA226::IResult<T>;

IResult<> INA226::update(){
    if(const auto res = this->read_reg(busVoltageReg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(currentReg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(powerReg);
        res.is_err()) return res;

    return Ok();
    // ;
    // this->read_reg(configReg);
    // DEBUG_PRINTLN(std::bitset<16>(configReg.as_val()));
    // read_burst(busVoltageReg.address, &busVoltageReg.as_ref(), 3).unwrap();
}


IResult<> INA226::write_reg(const RegAddress addr, const uint16_t data){
    return IResult<>(i2c_drv_.write_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_EN>("write error", uint8_t(addr), data);
}

IResult<> INA226::read_reg(const RegAddress addr, uint16_t & data){
    return IResult<>(i2c_drv_.read_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_EN>("read error", uint8_t(addr), data);
}

IResult<> INA226::read_reg(const RegAddress addr, int16_t & data){
    return IResult<>(i2c_drv_.read_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_EN>("read error", uint8_t(addr), data);
}
IResult<> INA226::init(const uint mohms, const uint max_current_a){
    configReg.rst = 0b0;
    configReg.__resv__ = 0b100;

    INA226_DEBUG("init");
    
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = reset();
        res.is_err()) return res;
    if(const auto res = set_average_times(16);
        res.is_err()) return res;
    if(const auto res = set_bus_conversion_time(ConversionTime::_140us);
        res.is_err()) return res;
    if(const auto res = set_shunt_conversion_time(ConversionTime::_140us);
        res.is_err()) return res;
    if(const auto res = enable_bus_voltage_measure();
        res.is_err()) return res;
    if(const auto res = enable_continuous_measure();
        res.is_err()) return res;
    if(const auto res = enable_shunt_voltage_measure();
        res.is_err()) return res;
    if(const auto res = config(mohms, max_current_a);
        res.is_err()) return res;
    return Ok();
}

IResult<> INA226::config(const uint mohms, const uint max_current_a){
    INA226_DEBUG(mohms, max_current_a);
    
    current_lsb_ma = real_t(int(max_current_a) * 1000) >> 15;
    // INA226_DEBUG(current_lsb_ma, mohms * max_current_a);
    const auto val = int(0.00512 * 32768 * 1000) / (mohms * max_current_a);
    // PANIC(calibrationReg.as_val());
    calibrationReg.as_ref() = int16_t(val);
    // PANIC(calibrationReg.as_val(), val);
    return write_reg(calibrationReg);
}

IResult<> INA226::set_average_times(const uint16_t times){

    uint8_t temp = CTZ(times);
    uint8_t temp2;

    if(times <= 64){
        temp2 = temp / 2;
    }else{
        temp2 = 4 + (temp - 7); 
    }

    configReg.averageMode = temp2;
    return write_reg((configReg));
}

IResult<real_t> INA226::get_voltage(){
    return Ok(busVoltageReg.as_val() * voltage_lsb_mv / 1000);
    // return busVoltageReg.as_val();
}

IResult<int> INA226::get_shunt_voltage_uv(){
    return Ok((shuntVoltageReg << 1) + (shuntVoltageReg >> 1));
}

IResult<real_t> INA226::get_shunt_voltage(){
    const int uv = ({
        const auto res = get_shunt_voltage_uv();
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    return Ok(real_t(uv / 100) / 10000);
}

IResult<real_t> INA226::get_current(){
    return Ok(currentReg.as_val() * current_lsb_ma / 1000);
    // return currentReg.as_val() ;
}

IResult<real_t> INA226::get_power(){
    return Ok(powerReg.as_val() * current_lsb_ma / 40);
    // return powerReg.as_val();
}

IResult<> INA226::set_average_times(const AverageTimes times){
    configReg.averageMode = uint8_t(times);
    return write_reg(configReg);
}

IResult<> INA226::set_bus_conversion_time(const ConversionTime time){
    configReg.busVoltageConversionTime = uint8_t(time);
    return write_reg(configReg);
}

IResult<> INA226::set_shunt_conversion_time(const ConversionTime time){
    configReg.shuntVoltageConversionTime = uint8_t(time);
    return write_reg(configReg);
}

IResult<> INA226::reset(){
    configReg.rst = 1;
    const auto res = write_reg(configReg);
    configReg.rst = 0;
    return res;
}

IResult<> INA226::enable_shunt_voltage_measure(const Enable en){
    configReg.shuntVoltageEnable = en == EN;
    return write_reg(configReg);
}

IResult<> INA226::enable_bus_voltage_measure(const Enable en){
    configReg.busVoltageEnable = en == EN;
    return write_reg(configReg);
}

IResult<> INA226::enable_continuous_measure(const Enable en){
    configReg.continuos = en == EN;
    return write_reg(configReg);
}

IResult<> INA226::enable_alert_latch(const Enable en){
    maskReg.alertLatchEnable = en == EN;
    return write_reg(maskReg);
}

IResult<> INA226::validate(){
    if(const auto res = i2c_drv_.validate(); res.is_err()){
        INA226_ASSERT(false, "INA226 i2c lost");
        return Err(Error::HalError(res.unwrap_err()));
    }

    if(const auto res = this->read_reg(chipIDReg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(manufactureIDReg);
        res.is_err()) return res;

    if((chipIDReg != VALID_CHIP_ID)) 
        return CHECK_ERR(Err(Error::ChipIdVerifyFailed));
    if((manufactureIDReg != VALID_MANU_ID)) 
        return CHECK_ERR(Err(Error::ManuIdVerifyFailed));

    return Ok();
}