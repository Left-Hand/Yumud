#ifndef __INA226_HPP__
#define __INA226_HPP__

#include "device_defs.h"
#include "types/real.hpp"

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
        manufactureID = 0x08,
        chipID = 0x09,
    };

    struct ConfigReg:public Reg16{
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

    // struct ShuntVoltageReg:public Reg16{
    //     REG16_BEGIN
    //     REG16_END
    // };

    // struct BusVoltageReg:public Reg16{
    //     REG16_BEGIN
    //     REG16_END
    // };

    // struct PowerReg:public Reg16{
    //     REG16_BEGIN
    //     REG16_END
    // };

    // struct CurrentReg:public Reg16{
    //     REG16_BEGIN
    //     REG16_END
    // };

    // struct CalibrationReg:public Reg16{
    //     REG16_BEGIN
    //     REG16_END
    // };

    struct MaskReg:public Reg16{
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

    // struct AlertLimitReg:public Reg16{
    //     REG16_BEGIN
    //     REG16_END
    // };

    // struct ManufactureIDReg:public Reg16{
    //     REG16_BEGIN
    //     REG16_END
    // };

    // struct ChipIDReg:public Reg16{
    //     REG16_BEGIN
    //     REG16_END
    // };

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

    void requestRegData(const RegAddress & regAddress, uint8_t * data_ptr, const size_t len){
        bus_drv.readPool((uint8_t)regAddress, data_ptr, 2, len);
    }
public:
    INA226(I2cDrv & _i2c_drv):bus_drv(_i2c_drv){};

    void update(){

    }



    void init(const real_t & ohms, const real_t & max_current_a){
        setAverageTimes(16);
        setBusConversionTime(BusVoltageConversionTime::ms1_1);
        setShuntConversionTime(ShuntVoltageConversionTime::ms1_1);
        enableBusVoltageMeasure();
        enableContinuousMeasure();
        enableShuntVoltageMeasure();

        currentLsb = max_current_a * ((1000000.0) / 32767.0);
	    real_t theCal = (0.00512 * 32768) / (ohms * max_current_a);
        calibrationReg = (uint16_t)theCal;
        writeReg(RegAddress::calibration, calibrationReg);
    }

    uint16_t isValid(){
        static constexpr uint16_t valid_manu_id = 0x5449;
        static constexpr uint16_t valid_chip_id = 0x2260;

        readReg(RegAddress::chipID, chipIDReg);
        readReg(RegAddress::manufactureID, manufactureIDReg);

        return chipIDReg == valid_chip_id && manufactureIDReg == valid_manu_id;
        // return 
        // return chipIDReg.data;
    }

    real_t getVoltage(){
        readReg(RegAddress::busVoltage, busVoltageReg);
        return busVoltageReg * voltageLsb;
    }

    real_t getCurrent(){
        readReg(RegAddress::current, *(uint16_t *)&currentReg);
        return currentReg * currentLsb;
    }

    real_t getPower(){
        readReg(RegAddress::power, powerReg);
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