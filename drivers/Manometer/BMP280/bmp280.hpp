
#pragma once

#include "drivers/device_defs.h"
#include "sys/math/real.hpp"

// #define BMP280_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#define BMP280_DEBUG(...)

namespace ymd::drivers{

class BMP280{
public:
    enum class Mode:uint8_t{
        Sleep, Single, Cont = 0x03
    };

    enum class TempratureSampleMode:uint8_t{
        SKIP, Bit16, Bit17, Bit18, Bit19, Bit20 = 0x07
    };

    enum class PressureSampleMode:uint8_t{
        SKIP, Bit16, Bit17, Bit18, Bit19, Bit20 = 0x07
    };

    enum class DataRate:uint8_t{
        HZ200, HZ16, HZ8, HZ4, HZ2, HZ1, HZ0_5, HZ0_25
    };

    enum class FilterCoefficient{
        OFF, FC2, FC4, FC8, FC16
    };

protected:
    hal::I2cDrv i2c_drv_;

    scexpr uint8_t default_i2c_addr = 0xec;
    scexpr uint8_t valid_chipid = 0x58;
    scexpr uint8_t reset_key = 0xB6;

    struct ChipIDReg:public Reg8<>{};

    struct ResetReg:public Reg8<>{};

    struct StatusReg:public Reg8<>{
        uint8_t im:1;
        uint8_t __resv1__:2;
        uint8_t busy:1;
        uint8_t __resv2__:4;
    };

    struct CtrlReg:public Reg8<>{
        uint8_t mode:2;
        uint8_t osrs_p:3;
        uint8_t osrs_t:3;
    };

    struct ConfigReg:public Reg8<>{
        uint8_t spi3_en:1;
        uint8_t __resv__:1;
        uint8_t filter:3;
        uint8_t t_sb:3;
    };

    struct PressureReg:public Reg16<>{uint16_t:16;};
    struct PressureXReg:public Reg8<>{uint8_t:8;};
    struct TemperatureReg:public Reg16<>{uint16_t:16;};
    struct TemperatureXReg:public Reg8<>{uint8_t:8;};

    enum class RegAddress:uint8_t{
        DigT1 = 0x88,
        DigT2 = 0x8A,
        DigT3 = 0x8C,
        DigP1 = 0x8E,
        DigP2 = 0x90,
        DigP3 = 0x92,
        DigP4 = 0x94,
        DigP5 = 0x96,
        DigP6 = 0x98,
        DigP7 = 0x9A,
        DigP8 = 0x9C,
        DigP9 = 0x9E,
        ChipID=0xD0,
        Reset=0xE0,
        Status=0xF3,
        Ctrl=0xF4,
        Config=0xF5,
        Pressure=0xF7,
        PressureX=0xF9,
        Temperature=0xFA,
        TemperatureX=0xFC
    };

    uint16_t digT1;
    int16_t digT2;
    int16_t digT3;
    uint16_t digP1;
    int16_t digP2;
    int16_t digP3;
    int16_t digP4;
    int16_t digP5;
    int16_t digP6;
    int16_t digP7;
    int16_t digP8;
    int16_t digP9;
    ChipIDReg chipIDReg;
    ResetReg resetReg;
    StatusReg statusReg;
    CtrlReg ctrlReg;
    ConfigReg configReg;
    PressureReg pressureReg;
    PressureXReg pressureXReg;
    TemperatureReg temperatureReg;
    TemperatureXReg temperatureXReg;

    void writeReg(const RegAddress addr, const uint16_t data){
        i2c_drv_.writeReg(uint8_t(addr), data, LSB);
    }

    void readReg(const RegAddress addr, uint16_t & data){
        i2c_drv_.readReg(uint8_t(addr), data, LSB);
    }

    void writeReg(const RegAddress addr, const uint8_t data){
        i2c_drv_.writeReg(uint8_t(addr), data);
    }

    void readReg(const RegAddress addr, uint8_t & data){
        i2c_drv_.readReg(uint8_t(addr), data);
        // BMP280_DEBUG(uint8_t(addr), (uint8_t)data);
    }

    void requestPool(const RegAddress addr, uint8_t * datas, uint8_t size, uint8_t len){
        i2c_drv_.readMulti(uint8_t(addr), datas, len);
    }

    uint32_t getPressureData(){
        uint32_t pressureData = 0;
        readReg(RegAddress::Pressure, pressureReg);
        pressureData = pressureReg << 4;
        readReg(RegAddress::PressureX, pressureXReg);
        pressureData |= pressureXReg >> 4;
        // BMP280_DEBUG("PressureData:", pressureData);
        return pressureData;
    }

    uint32_t getTemperatureData(){
        uint32_t temperatureData = 0;
        readReg(RegAddress::Temperature, temperatureReg);
        temperatureData = temperatureReg << 4;
        readReg(RegAddress::TemperatureX, temperatureXReg);
        temperatureData |= temperatureXReg >> 4;
        // BMP280_DEBUG("TempratureData:", temperatureData);
        return temperatureData;
    }

public:
    BMP280(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    BMP280(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    BMP280(hal::I2c & _i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(hal::I2cDrv(_i2c, addr)){;}
    ~BMP280(){;}

    bool isChipValid();

    void setTempratureSampleMode(const TempratureSampleMode tempMode);

    void setPressureSampleMode(const PressureSampleMode pressureMode);
    void setMode(const Mode mode);

    void setDataRate(const DataRate dataRate);

    void setFilterCoefficient(const FilterCoefficient filterCoeff);

    void reset();

    bool isIdle();

    void enableSpi3(const bool en = true);

    void getPressure(int32_t & pressure);

    void init();
};

}