#ifndef __INA226_HPP__
#define __INA226_HPP__

#include "../drivers/device_defs.h"
#include "../types/real.hpp"
#include <bit>

#ifdef INA226_DEBUG
#undef INA226_DEBUG
#define INA226_DEBUG(...) DEBUG_LOG(...)
#else 
#define INA226_DEBUG(...)
#endif


class INA226 {
public:

    enum class AverageTimes:uint8_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class ConversionTime:uint8_t{
        _140us = 0, _204us, _332us, _588us, _1_1ms, _2_116_ms, _4_156ms, _8_244ms
    };

protected:
    I2cDrv i2c_drv;
    
    real_t currentLsb = real_t(0.0002);
    real_t voltageLsb = real_t(0.00125);

    enum class RegAddress:uint8_t{
        Config = 0x00,
        shuntVoltage = 0x01,
        busVoltage = 0x02,
        power = 0x03,
        current = 0x04,
        calibration = 0x05,
        mask = 0x06,
        alertLimit = 0x07,
        manufactureID = 0xFE,
        chipID = 0xFF,
    };

    struct ConfigReg{
        uint16_t shuntVoltageEnable :1;
        uint16_t busVoltageEnable :1;
        uint16_t continuos :1;
        uint16_t shuntVoltageConversionTime:3;
        uint16_t busVoltageConversionTime:3;
        uint16_t averageMode:3;
        uint16_t __resv__:3;
        uint16_t rst:1;
    };

    struct MaskReg{
        uint16_t alertLatchEnable:1;
        uint16_t alertPolarity:1;
        uint16_t mathOverflow:1;
        uint16_t convReadyFlag:1;
        uint16_t alertFlag:1;
        uint16_t __resv__:5;
        uint16_t convReady:1;
        uint16_t powerOverlimit:1;
        uint16_t busUnderVoltage:1;
        uint16_t busOverVoltage:1;
        uint16_t shuntUnderVoltage:1;
        uint16_t shuntOverVoltage:1;
    };

    struct{
        ConfigReg configReg;
        uint16_t shuntVoltageReg;
        uint16_t busVoltageReg;
        uint16_t powerReg;
        int16_t currentReg;
        int16_t calibrationReg;
        MaskReg maskReg;
        uint16_t alertLimitReg;
        uint16_t manufactureIDReg;
        uint16_t chipIDReg;
    };

    void writeReg(const RegAddress regAddress, const uint16_t regData){
        i2c_drv.writeReg((uint8_t)regAddress, *(uint16_t *) &regData);
    }

    void readReg(const RegAddress regAddress, uint16_t & regData){
        i2c_drv.readReg((uint8_t)regAddress, (uint16_t &)regData);
    }

    void requestPool(const RegAddress regAddress, void * data_ptr, const size_t len){
        i2c_drv.readPool((uint8_t)regAddress, (uint16_t *)data_ptr, len, LSB);
    }
public:
    static constexpr uint8_t default_i2c_addr = 0x80;

    INA226(const I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){;}
    INA226(I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){;}
    INA226(I2c & _i2c, const uint8_t _addr = default_i2c_addr):i2c_drv(I2cDrv(_i2c, _addr)){};


    void update();


    void init(const real_t ohms, const real_t max_current_a);

    void setAverageTimes(const uint16_t times);

    bool verify();

    real_t getVoltage(){
        return busVoltageReg * voltageLsb;
    }

    int getShuntVoltageuV(){
        return((shuntVoltageReg << 1) + (shuntVoltageReg >> 1));
    }

    real_t getShuntVoltage(){
        auto uv = getShuntVoltageuV();
        return real_t(uv / 100) / 10000;
    }

    real_t getCurrent(){
        return currentReg * currentLsb;
    }

    real_t getPower(){
        return powerReg * currentLsb * 25;
    }



    void setAverageTimes(const AverageTimes times){
        configReg.averageMode = uint8_t(times);
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void setBusConversionTime(const ConversionTime &time){
        configReg.busVoltageConversionTime = uint8_t(time);
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void setShuntConversionTime(const ConversionTime &time){
        configReg.shuntVoltageConversionTime = uint8_t(time);
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void reset(){
        configReg.rst = 1;
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
        configReg.rst = 0;
    }

    void enableShuntVoltageMeasure(const bool en = true){
        configReg.shuntVoltageEnable = en;
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void enableBusVoltageMeasure(const bool en = true){
        configReg.busVoltageEnable = en;
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void enableContinuousMeasure(const bool en = true){
        configReg.continuos = en;
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void enableAlertLatch(const bool en = true){
        maskReg.alertLatchEnable = en;
        writeReg(RegAddress::mask, std::bit_cast<uint16_t>(maskReg));
    }
};

#endif