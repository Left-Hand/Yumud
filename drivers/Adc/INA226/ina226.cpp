#include "ina226.hpp"

using namespace ymd::drivers;

void INA226::update(){
    // requestPool(RegAddress::shuntVoltage, &shuntVoltageReg, 2 * 4);
    // requestPool(RegAddress::busVoltage, &busVoltageReg, 3);
    readReg(RegAddress::busVoltage, busVoltageReg);
    readReg(RegAddress::current, *(uint16_t *)&currentReg);
    readReg(RegAddress::power, powerReg);
}


void INA226::init(const real_t ohms, const real_t max_current_a){
    configReg.rst = 0b0;
    configReg.__resv__ = 0b100;

    setAverageTimes(16);
    setBusConversionTime(ConversionTime::_140us);
    setShuntConversionTime(ConversionTime::_140us);
    enableBusVoltageMeasure();
    enableContinuousMeasure();
    enableShuntVoltageMeasure();

    config(ohms, max_current_a);

    delay(10);
}

void INA226::config(const real_t ohms, const real_t max_current_a){
    currentLsb = max_current_a * real_t(1/32768.0);
    calibrationReg = (int16_t)real_t(real_t(0.00512 * 32768) / (ohms * max_current_a));
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