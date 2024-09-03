#ifndef __INA226_HPP__
#define __INA226_HPP__

#include "../drivers/device_defs.h"
#include "../types/real.hpp"

#ifdef INA226_DEBUG
#define INA226_DEBUG(...) DEBUG_LOG(...)
#endif


class INA226 {
public:
    enum class AverageTimes:uint8_t{
        Sample1 = 0,
        Sample4 = 1,
        Sample16 = 2,
        Sample64 = 3,
        Sample128 = 4,
        Sample256 = 5,
        Sample512 = 6,
        Sample1024 = 7
    };

    enum class BusVoltageConversionTime:uint8_t{
        us140 = 0, us204, us332, us588, ms1_1, ms2_116, ms4_156, ms8_244
    };

    enum class ShuntVoltageConversionTime:uint8_t{
        us140 = 0, us204, us332, us588, ms1_1, ms2_116, ms4_156, ms8_244
    };

    static constexpr uint8_t default_id = 0x80; 
protected:
    I2cDrv & bus_drv;
    
    real_t currentLsb = 0.0002;
    real_t voltageLsb = 0.00125;

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
        REG16_BEGIN
        uint16_t shuntVoltageEnable :1;
        uint16_t busVoltageEnable :1;
        uint16_t continuos :1;
        uint16_t shuntVoltageConversionTime:3;
        uint16_t busVoltageConversionTime:3;
        uint16_t averageMode:3;
        uint16_t __resv__:3;
        uint16_t rst:1;
        REG16_END
    };

    struct MaskReg{
        REG16_BEGIN
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
        REG16_END
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

    void writeReg(const RegAddress & regAddress, const uint16_t & regData){
        bus_drv.writeReg((uint8_t)regAddress, *(uint16_t *) &regData);
    }

    void readReg(const RegAddress & regAddress, uint16_t & regData){
        bus_drv.readReg((uint8_t)regAddress, (uint16_t &)regData);
    }

    void requestPool(const RegAddress & regAddress, void * data_ptr, const size_t len){
        bus_drv.readPool((uint8_t)regAddress, (uint16_t *)data_ptr, len);
    }
public:
    INA226(I2cDrv & _i2c_drv):bus_drv(_i2c_drv){};

    void update(){
        // requestPool(RegAddress::shuntVoltage, &shuntVoltageReg, 2 * 4);
        readReg(RegAddress::busVoltage, busVoltageReg);
        readReg(RegAddress::current, *(uint16_t *)&currentReg);
        readReg(RegAddress::power, powerReg);
    }


    void init(const real_t ohms, const real_t max_current_a){
        configReg.rst = 0b0;
        configReg.__resv__ = 0b100;

        setAverageTimes(16);
        setBusConversionTime(BusVoltageConversionTime::ms1_1);
        setShuntConversionTime(ShuntVoltageConversionTime::ms1_1);
        enableBusVoltageMeasure();
        enableContinuousMeasure();
        enableShuntVoltageMeasure();

        currentLsb = max_current_a * real_t(1/32768.0);
        calibrationReg = (uint16_t)real_t(real_t(0.00512 * 32768) / (ohms * max_current_a));
        writeReg(RegAddress::calibration, calibrationReg);

        delay(10);
    }

    uint16_t isValid(){
        static constexpr uint16_t valid_manu_id = 0x5449;
        static constexpr uint16_t valid_chip_id = 0x2260;

        readReg(RegAddress::chipID, chipIDReg);
        readReg(RegAddress::manufactureID, manufactureIDReg);

        return chipIDReg == valid_chip_id && manufactureIDReg == valid_manu_id;
    }

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

    void setAverageTimes(const uint16_t & times){
        uint8_t temp = CTZ(times);
        uint8_t temp2;

        if(times <= 64){
            temp2 = temp / 2;
        }else{
            temp2 = 4 + (temp - 7); 
        }

        configReg.averageMode = temp2;
        DEBUG_VALUE(temp2);
        writeReg(RegAddress::Config, configReg.data);
    }

    void setAverageTimes(const AverageTimes & times){
        configReg.averageMode = uint8_t(times);
        writeReg(RegAddress::Config, configReg.data);
    }

    void setBusConversionTime(const BusVoltageConversionTime & time){
        configReg.busVoltageConversionTime = uint8_t(time);
        writeReg(RegAddress::Config, configReg.data);
    }

    void setShuntConversionTime(const ShuntVoltageConversionTime & time){
        configReg.shuntVoltageConversionTime = uint8_t(time);
        writeReg(RegAddress::Config, configReg.data);
    }

    void reset(){
        configReg.rst = 1;
        writeReg(RegAddress::Config, configReg.data);
        configReg.rst = 0;
    }

    void enableShuntVoltageMeasure(const bool & en = true){
        configReg.shuntVoltageEnable = en;
        writeReg(RegAddress::Config, configReg.data);
    }

    void enableBusVoltageMeasure(const bool & en = true){
        configReg.busVoltageEnable = en;
        writeReg(RegAddress::Config, configReg.data);
    }

    void enableContinuousMeasure(const bool & en = true){
        configReg.continuos = en;
        writeReg(RegAddress::Config, configReg.data);
    }

    void enableAlertLatch(const bool & en = true){
        maskReg.alertLatchEnable = en;
        writeReg(RegAddress::mask, maskReg.data);
    }
};

#endif