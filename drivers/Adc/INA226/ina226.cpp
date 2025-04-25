#include "ina226.hpp"

#include "core/debug/debug.hpp"

#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


// #define INA226_DEBUG_ON 1
#define INA226_DEBUG_ON 0

#ifdef INA226_DEBUG_ON
#define INA226_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA226_PANIC(...) PANIC(__VA_ARGS__)
#define INA226_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}

#define WRITE_REG(reg) this->write_reg(reg.address, reg.as_val()).loc().expect();
#define READ_REG(reg) this->read_reg(reg.address, reg.as_ref()).loc().expect();
#else
#define INA226_DEBUG(...)
#define INA226_PANIC(...)  PANIC_NSRC()
#define INA226_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define WRITE_REG(reg) this->write_reg(reg.address, reg.as_val()).unwrap();
#define READ_REG(reg) this->read_reg(reg.address, reg.as_ref()).unwrap();
#endif


using BusResult = INA226::BusResult;

void INA226::update(){
    READ_REG(busVoltageReg);
    // delay(1);
    READ_REG(currentReg);
    // READ_REG(shuntVoltageReg);
    READ_REG(powerReg);
    // READ_REG(configReg);
    // DEBUG_PRINTLN(std::bitset<16>(configReg.as_val()));
    // read_burst(busVoltageReg.address, &busVoltageReg.as_ref(), 3).unwrap();
}


BusResult INA226::write_reg(const RegAddress addr, const uint16_t data){
    return BusResult(i2c_drv_.write_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_ON>("write error", uint8_t(addr), data);
}

BusResult INA226::read_reg(const RegAddress addr, uint16_t & data){
    return BusResult(i2c_drv_.read_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_ON>("read error", uint8_t(addr), data);
}

BusResult INA226::read_reg(const RegAddress addr, int16_t & data){
    return BusResult(i2c_drv_.read_reg(uint8_t(addr), data, MSB))
        .check_if<INA226_DEBUG_ON>("read error", uint8_t(addr), data);
}

BusResult INA226::read_burst(const RegAddress addr, uint16_t * p_data, const size_t len){
    return i2c_drv_.read_burst(uint8_t(addr), std::span(p_data, len), LSB);
}


void INA226::init(const uint mohms, const uint max_current_a){
    configReg.rst = 0b0;
    configReg.__resv__ = 0b100;

    INA226_DEBUG("init");

    reset();
    INA226_ASSERT(verify(), "INA226 not found");
    
    set_average_times(16);
    set_bus_conversion_time(ConversionTime::_140us);
    set_shunt_conversion_time(ConversionTime::_140us);
    enable_bus_voltage_measure();
    enable_continuous_measure();
    enable_shunt_voltage_measure();
    
    INA226_DEBUG("config");

    config(mohms, max_current_a);

}

void INA226::config(const uint mohms, const uint max_current_a){
    INA226_DEBUG(mohms, max_current_a);
    
    current_lsb_ma = real_t(int(max_current_a) * 1000) >> 15;
    // INA226_DEBUG(current_lsb_ma, mohms * max_current_a);
    const auto val = int(0.00512 * 32768 * 1000) / (mohms * max_current_a);
    // PANIC(calibrationReg.as_val());
    calibrationReg.as_ref() = int16_t(val);
    // PANIC(calibrationReg.as_val(), val);
    WRITE_REG(calibrationReg);
}

void INA226::set_average_times(const uint16_t times){

    uint8_t temp = CTZ(times);
    uint8_t temp2;

    if(times <= 64){
        temp2 = temp / 2;
    }else{
        temp2 = 4 + (temp - 7); 
    }

    configReg.averageMode = temp2;
    WRITE_REG((configReg));
}

real_t INA226::get_voltage(){
    return busVoltageReg.as_val() * voltage_lsb_mv / 1000;
    // return busVoltageReg.as_val();
}

int INA226::get_shunt_voltage_uv(){
    return((shuntVoltageReg << 1) + (shuntVoltageReg >> 1));
}

real_t INA226::get_shunt_voltage(){
    auto uv = get_shunt_voltage_uv();
    return real_t(uv / 100) / 10000;
}

real_t INA226::get_current(){
    return currentReg.as_val() * current_lsb_ma / 1000;
    // return currentReg.as_val() ;
}

real_t INA226::get_power(){
    return powerReg.as_val() * current_lsb_ma / 40;
    // return powerReg.as_val();
}

void INA226::set_average_times(const AverageTimes times){
    configReg.averageMode = uint8_t(times);
    WRITE_REG(configReg);
}

void INA226::set_bus_conversion_time(const ConversionTime time){
    configReg.busVoltageConversionTime = uint8_t(time);
    WRITE_REG(configReg);
}

void INA226::set_shunt_conversion_time(const ConversionTime time){
    configReg.shuntVoltageConversionTime = uint8_t(time);
    WRITE_REG(configReg);
}

void INA226::reset(){
    configReg.rst = 1;
    WRITE_REG(configReg);
    configReg.rst = 0;
}

void INA226::enable_shunt_voltage_measure(const bool en){
    configReg.shuntVoltageEnable = en;
    WRITE_REG(configReg);
}

void INA226::enable_bus_voltage_measure(const bool en){
    configReg.busVoltageEnable = en;
    WRITE_REG(configReg);
}

void INA226::enable_continuous_measure(const bool en){
    configReg.continuos = en;
    WRITE_REG(configReg);
}

void INA226::enable_alert_latch(const bool en){
    maskReg.alertLatchEnable = en;
    WRITE_REG(maskReg);
}

bool INA226::verify(){
    scexpr uint16_t valid_manu_id = 0x5449;
    scexpr uint16_t valid_chip_id = 0x2260;
    
    INA226_ASSERT(i2c_drv_.verify().is_ok(), "INA226 i2c lost");

    READ_REG(chipIDReg);
    READ_REG(manufactureIDReg);

    return INA226_ASSERT(
        (chipIDReg == valid_chip_id and manufactureIDReg == valid_manu_id), 
        "INA226 who am i failed", std::showbase, std::hex, chipIDReg.as_val(), manufactureIDReg.as_val()
    );
}