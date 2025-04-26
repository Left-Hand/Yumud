#include "ina226.hpp"

#include "core/debug/debug.hpp"

#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


#define INA226_DEBUG_ON 1
// #define INA226_DEBUG_ON 0

#if INA226_DEBUG_ON
#define INA226_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA226_PANIC(...) PANIC(__VA_ARGS__)
#define INA226_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define INA226_ERROR(err, ...) []{PANIC(__VA_ARGS__); return Err(err);}() 
#else
#define INA226_DEBUG(...)
#define INA226_PANIC(...)  PANIC_NSRC()
#define INA226_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define INA226_ERROR(err, ...) {Err(err)}
#endif


template<typename T = void>
using IResult = INA226::IResult<T>;

IResult<> INA226::update(){
    return this->read_reg(busVoltageReg)
    // delay(1);
    | this->read_reg(currentReg)
    // this->read_reg(shuntVoltageReg);
    | this->read_reg(powerReg)
    ;
    // this->read_reg(configReg);
    // DEBUG_PRINTLN(std::bitset<16>(configReg.as_val()));
    // read_burst(busVoltageReg.address, &busVoltageReg.as_ref(), 3).unwrap();
}


IResult<> INA226::write_reg(const RegAddress addr, const uint16_t data){
    return IResult<>(i2c_drv_.write_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_ON>("write error", uint8_t(addr), data);
}

IResult<> INA226::read_reg(const RegAddress addr, uint16_t & data){
    return IResult<>(i2c_drv_.read_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_ON>("read error", uint8_t(addr), data);
}

IResult<> INA226::read_reg(const RegAddress addr, int16_t & data){
    return IResult<>(i2c_drv_.read_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_ON>("read error", uint8_t(addr), data);
}

// IResult<> INA226::read_burst(const RegAddress addr, uint16_t * p_data, const size_t len){
//     return IResult(i2c_drv_.read_burst(uint8_t(addr), std::span(p_data, len), LSB));
// }


IResult<> INA226::init(const uint mohms, const uint max_current_a){
    configReg.rst = 0b0;
    configReg.__resv__ = 0b100;

    INA226_DEBUG("init");
    
    return verify()
    | reset()
    | set_average_times(16)
    | set_bus_conversion_time(ConversionTime::_140us)
    | set_shunt_conversion_time(ConversionTime::_140us)
    | enable_bus_voltage_measure()
    | enable_continuous_measure()
    | enable_shunt_voltage_measure()
    | config(mohms, max_current_a)
    ;
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

IResult<void> INA226::set_average_times(const AverageTimes times){
    configReg.averageMode = uint8_t(times);
    return write_reg(configReg);
}

IResult<void> INA226::set_bus_conversion_time(const ConversionTime time){
    configReg.busVoltageConversionTime = uint8_t(time);
    return write_reg(configReg);
}

IResult<void> INA226::set_shunt_conversion_time(const ConversionTime time){
    configReg.shuntVoltageConversionTime = uint8_t(time);
    return write_reg(configReg);
}

IResult<void> INA226::reset(){
    configReg.rst = 1;
    const auto res = write_reg(configReg);
    configReg.rst = 0;
    return res;
}

IResult<void> INA226::enable_shunt_voltage_measure(const bool en){
    configReg.shuntVoltageEnable = en;
    return write_reg(configReg);
}

IResult<void> INA226::enable_bus_voltage_measure(const bool en){
    configReg.busVoltageEnable = en;
    return write_reg(configReg);
}

IResult<void> INA226::enable_continuous_measure(const bool en){
    configReg.continuos = en;
    return write_reg(configReg);
}

IResult<void> INA226::enable_alert_latch(const bool en){
    maskReg.alertLatchEnable = en;
    return write_reg(maskReg);
}

IResult<> INA226::verify(){
    if(const auto phy_v = i2c_drv_.verify(); phy_v.is_err()){
        INA226_ASSERT(false, "INA226 i2c lost");
        return Err(Error::BusError(phy_v));
    }

    if(const auto res = this->read_reg(chipIDReg); res.is_err()) return res;
    if(const auto res = this->read_reg(manufactureIDReg); res.is_err()) return res;

    if((chipIDReg != VALID_CHIP_ID)) return INA226_ERROR(Error::ChipIdVerifyFailed);
    if((manufactureIDReg == VALID_MANU_ID)) return INA226_ERROR(Error::ManuIdVerifyFailed);

    return Ok();
}