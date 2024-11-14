#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"
#include <tuple>

namespace ymd::drivers{

class QMC5883L:public Magnetometer{
public:
    enum class DataRate:uint8_t{
        DR10, DR50, DR100, DR200
    };

    enum class OverSampleRatio:uint8_t{
        OSR512, OSR256, OSR128, OSR64
    };

    enum class FullScale:uint8_t{
        FS2G, FS8G
    };

    enum class Mode:uint8_t{
        Single,Continuous
    };

    scexpr uint8_t default_i2c_addr = 0x1a;
protected:
    I2cDrv i2c_drv_;

    real_t fs;
    uint8_t ovsfix = 0;

    struct MagXReg:public Reg16{
        uint16_t data;
    };

    struct MagYReg:public Reg16{
        uint16_t data;
    };

    struct MagZReg:public Reg16{
        uint16_t data;
    };

    struct StatusReg:public Reg8{
        uint8_t ready:1;
        uint8_t ovl:1;
        uint8_t lock:1;
        uint8_t __resv__:5;
    };

    struct TemperatureReg:public Reg16{
        uint16_t data;
    };

    struct ConfigAReg:public Reg8{
        
        uint8_t measureMode:2;
        uint8_t dataRate:2;
        uint8_t fullScale:2;
        uint8_t OverSampleRatio:2;
        
    };

    struct ConfigBReg:public Reg8{
        
        uint8_t intEn:1;
        uint8_t __resv__:5;
        uint8_t rol:1;
        uint8_t srst:1;
        
    };

    struct ResetPeriodReg:public Reg8{
        using Reg8::operator=;
        uint8_t data;
    };

    struct ChipIDReg:public Reg8{
        uint8_t data;
    };

    enum class RegAddress:uint8_t{
        MagX = 0x00,
        MagY = 0x02,
        MagZ = 0x04,
        Status = 0x06,
        Tempature = 0x07,
        ConfigA = 0x09,
        ConfigB = 0x0A,
        ResetPeriod = 0x0B,
        ChipID = 0x0D
    };

    struct{
        MagXReg magXReg;
        MagYReg magYReg;
        MagZReg magZReg;
        StatusReg statusReg;
        TemperatureReg temperatureReg;
        ConfigAReg configAReg;
        ConfigBReg configBReg;
        ResetPeriodReg resetPeriodReg;
        ChipIDReg chipIDReg;
    };

    void writeReg(const RegAddress regAddress, const uint16_t regData){
        i2c_drv_.writeReg((uint8_t)regAddress, regData, LSB);
    }

    void readReg(const RegAddress regAddress, uint16_t & regData){
        i2c_drv_.readReg((uint8_t)regAddress, regData, LSB);
    }

    void writeReg(const RegAddress regAddress, const uint8_t regData){
        i2c_drv_.writeReg((uint8_t)regAddress, regData, LSB);
    }

    void readReg(const RegAddress regAddress, uint8_t & regData){
        i2c_drv_.readReg((uint8_t)regAddress, regData, LSB);
    }

    void requestPool(const RegAddress regAddress, uint8_t * datas, uint8_t len){
        i2c_drv_.readMulti((uint8_t)regAddress, datas, len, LSB);
    }

    real_t From16BitToGauss(const uint16_t data){
        real_t guass;
        s16_to_uni(data, guass);
        guass *= fs;
        return guass;
    }

    void setFs(const FullScale FS){
        switch(FS){
        case FullScale::FS2G:
            fs = real_t(2);
            break;
        case FullScale::FS8G:
            fs = real_t(8);
            break;
        default:
            break;
        }
    }

    void setOvsfix(const OverSampleRatio OSR){
        switch(OSR){
        case OverSampleRatio::OSR512:
            ovsfix = 9;
            break;
        case OverSampleRatio::OSR256:
            ovsfix = 8;
            break;
        case OverSampleRatio::OSR128:
            ovsfix = 7;
            break;
        case OverSampleRatio::OSR64:
            ovsfix = 6;
            break;
        default:
            break;
        }
    }

    bool isIdle(){
        readReg(RegAddress::Status, statusReg);
        return statusReg.ready;
    }
public:
    QMC5883L(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    QMC5883L(I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    QMC5883L(I2c & bus):i2c_drv_(I2cDrv(bus, default_i2c_addr)){;}

    void init(){
        if(isChipValid()){
            setResetPeriod(1);
            enableContMode();
            setFullScale(FullScale::FS2G);
            setOverSampleRatio(OverSampleRatio::OSR512);
            setDataRate(DataRate::DR200);
        }
    }

    void enableContMode(const bool en = true){
        configAReg.measureMode = (uint8_t)(en);
        writeReg(RegAddress::ConfigA, configAReg);
    }

    void setDataRate(const DataRate rate){
        configAReg.dataRate = (uint8_t)rate;
        writeReg(RegAddress::ConfigA, configAReg);
    }

    void setFullScale(const FullScale fullscale){
        configAReg.fullScale = (uint8_t)fullscale;
        writeReg(RegAddress::ConfigA, configAReg);
        setFs(fullscale);
    }

    void setOverSampleRatio(const OverSampleRatio ratio){
        configAReg.OverSampleRatio = (uint8_t)ratio;
        writeReg(RegAddress::ConfigA, configAReg);
        setOvsfix(ratio);
    }

    void update(){
        bool done = isIdle();
        if(done){
            requestPool(RegAddress::MagX, (uint8_t *)&magXReg, 3);
        }
        // return done;
    }

    std::tuple<real_t, real_t, real_t> getMagnet() override {
        return std::make_tuple(
            From16BitToGauss(magXReg),
            From16BitToGauss(magYReg),
            From16BitToGauss(magZReg)
        );
    }
    bool isChipValid(){
        readReg(RegAddress::ChipID, chipIDReg);
        return (chipIDReg == 0xFF);
    }

    void setResetPeriod(const uint8_t resetPeriod){
        resetPeriodReg = resetPeriod;
        writeReg(RegAddress::ResetPeriod, resetPeriodReg);
    }

    void reset(){
        configBReg.srst = true;
        writeReg(RegAddress::ConfigB, resetPeriodReg);
        configBReg.srst = false;
        writeReg(RegAddress::ConfigB, resetPeriodReg);
    }

    void enableInterrupt(const bool en = true){
        configBReg.intEn = (uint8_t)(en);
        writeReg(RegAddress::ConfigB, configBReg);
    }



    bool isOverflow(){
        readReg(RegAddress::Status, statusReg);
        return statusReg.ovl;
    }

};


}