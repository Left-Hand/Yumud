#include "ina226.hpp"

#include "sys/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


// #define INA226_DEBUG

#ifdef INA226_DEBUG
#undef INA226_DEBUG
#define INA226_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA226_PANIC(...) PANIC(__VA_ARGS__)
#define INA226_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define INA226_DEBUG(...)
#define INA226_PANIC(...)  PANIC()
#define INA226_ASSERT(cond, ...) ASSERT(cond)
#endif

#define WRITE_REG(reg) this->writeReg(reg.address, reg);
#define READ_REG(reg) this->readReg(reg.address, reg);



void INA226::update(){
    readReg(RegAddress::busVoltage, busVoltageReg);
    readReg(RegAddress::current, *(uint16_t *)&currentReg);
    readReg(RegAddress::power, powerReg);
}


BusError INA226::writeReg(const RegAddress addr, const uint16_t data){
    INA226_DEBUG("w", uint8_t(addr), data);
    return i2c_drv.writeReg((uint8_t)addr, data, MSB);
}

BusError INA226::readReg(const RegAddress addr, uint16_t & data){
    INA226_DEBUG("r",uint8_t(addr), data);
    return i2c_drv.readReg((uint8_t)addr, data, MSB);
}

BusError INA226::requestPool(const RegAddress addr, uint16_t * p_data, const size_t len){
    INA226_DEBUG("p",uint8_t(addr), uint32_t(p_data), len);
    return i2c_drv.readMulti((uint8_t)addr, p_data, len, LSB);
}


void INA226::init(const uint mohms, const uint max_current_a){
    configReg.rst = 0b0;
    configReg.__resv__ = 0b100;

    INA226_DEBUG("init");

    INA226_ASSERT(verify());
    
    setAverageTimes(16);
    setBusConversionTime(ConversionTime::_140us);
    setShuntConversionTime(ConversionTime::_140us);
    enableBusVoltageMeasure();
    enableContinuousMeasure();
    enableShuntVoltageMeasure();
    
    INA226_DEBUG("config");

    config(mohms, max_current_a);

    delay(10);
}

void INA226::config(const uint mohms, const uint max_current_a){
    INA226_DEBUG(mohms, max_current_a);
    
    current_lsb_ma = real_t(int(max_current_a) * 1000) >> 15;
    // INA226_DEBUG(current_lsb_ma, mohms * max_current_a);
    calibrationReg = int16_t(int(0.00512 * 32768 * 1000) / (mohms * max_current_a));

    writeReg(RegAddress::calibration, calibrationReg);
}

void INA226::setAverageTimes(const uint16_t times){

    uint8_t temp = CTZ(times);
    uint8_t temp2;

    if(times <= 64){
        temp2 = temp / 2;
    }else{
        temp2 = 4 + (temp - 7); 
    }

    configReg.averageMode = temp2;
    writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
}

bool INA226::verify(){
    scexpr uint16_t valid_manu_id = 0x5449;
    scexpr uint16_t valid_chip_id = 0x2260;

    readReg(RegAddress::chipID, chipIDReg);
    readReg(RegAddress::manufactureID, manufactureIDReg);

    return chipIDReg == valid_chip_id and manufactureIDReg == valid_manu_id;
}