#ifndef __INA226_HPP__
#define __INA226_HPP__

#include "device_defs.h"
#include "types/real.hpp"

#ifdef INA226_DEBUG
#define INA226_DEBUG(...) DEBUG_LOG(...)
#endif


class INA226 {
public:
    enum class AverageMode:uint8_t{
        Sample1 = 0;
        Sample4 = 1;
        Sample16 = 2;
        Sample64 = 3;
        Sample128 = 4;
        Sample256 = 5;
        Sample512 = 6;
        Sample1024 = 7;
    };

    enum class BusVoltageConversionTime:uint8_t{
        us140 = 0, us204, us332, us588, ms1_1, ms2_116, ms4_156, ms8_244
    };

    enum class ShuntVoltageConversionTime:uint8_t{
        us140 = 0, us204, us332, us588, ms1_1, ms2_116, ms4_156, ms8_244
    };
protected:
    I2cDrv & i2c_drv;

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
    }

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

    struct ShuntVoltageReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct BusVoltageReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct PowerReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct CurrentReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct CalibrationReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

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

    struct AlertLimitReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct ManufactureIDReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct ChipIDReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct{
        ConfigReg configReg;
        ShuntVoltageReg shuntVoltageReg;
        BusVoltageReg busVoltageReg;
        PowerReg powerReg;
        CurrentReg currentReg;
        CalibrationReg calibrationReg;
        MaskReg maskReg;
        AlertLimitReg alertLimitReg;
        ManufactureIDReg manufactureIDReg;
        ChipIDReg chipIDReg;
    }
    void writeReg(const RegAddress & regAddress, const Reg16 & regData){
        bus_drv.writeReg((uint8_t)regAddress, *(uint16_t *) &regData);
    }

    void readReg(const RegAddress & regAddress, Reg16 & regData){
        bus_drv.readReg((uint8_t)regAddress, (uint16_t &)regData);
    }

    void requestRegData(const RegAddress & regAddress, uint8_t * data_ptr, const size_t len){
        bus_drv.readPool((uint8_t)regAddress, data_ptr, 2, len);
    }
public:
    uint16_t getChipId(){
        readReg(RegAddress::chipID, chipIDReg);
        return chipIDReg.data;
    }

    uint16_t getManufactureId(){
        readReg(RegAddress::manufactureID, manufactureIDReg);
        return manufactureIDReg.data;
    }
}
