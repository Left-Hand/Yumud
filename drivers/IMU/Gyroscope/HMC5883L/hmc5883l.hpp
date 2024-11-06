#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

namespace yumud::drivers{

class HMC5883L:public Magnetometer{
public:
    enum class DataRate:uint8_t{
        DR0_75, DR1_5, DR3, DR7_5, DR15, DR30, DR75
    };

    enum class SampleNumber:uint8_t{
        SN1, SN2, SN4, SN8
    };

    enum class MeasurementMode:uint8_t{
        Norm, Pos, Neg
    };

    enum class Gain:uint8_t{
        GL0_73, GL0_92, GL1_22, GL1_52, GL2_27, GL2_56, GL3_03, GL4_35
    };

    enum class Mode:uint8_t{
        Continuous, Single
    };

    scexpr uint8_t default_i2c_addr = 0x3d;
protected:
    I2cDrv i2c_drv_;

    real_t lsb;

    struct ConfigAReg:public Reg8{
        
        uint8_t measureMode:3;
        uint8_t dataRate:2;
        uint8_t sampleNumber:2;
        uint8_t __resv__:1;
        
    };

    struct ConfigBReg:public Reg8{
        
        uint8_t __resv__:5;
        uint8_t gain:3;
        
    };

    struct ModeReg:public Reg8{
        
        uint8_t mode:2;
        uint8_t __resv__:5;
        uint8_t hs:1;
        
    };

    struct StatusReg:public Reg8{
        
        uint8_t ready:1;
        uint8_t lock:1;
        uint8_t __resv__:6;
        
    };

    enum class RegAddress:uint8_t{
        ConfigA = 0x00,
        ConfigB = 0x01,
        Mode = 0x02,
        MagX = 0x03,
        MagY = 0x05,
        MagZ = 0x07,
        Status = 0x09,
        IDA = 10,
        IDB = 11,
        IDC = 12
    };

    struct{
        ConfigAReg configAReg;
        ConfigBReg configBReg;
        ModeReg modeReg;
        int16_t magXReg;
        int16_t magYReg;
        int16_t magZReg;
        StatusReg statusReg;
    };

    void writeReg(const RegAddress regAddress, const uint16_t regData){
        i2c_drv_.writeReg((uint8_t)regAddress, regData, MSB);
    }

    void readReg(const RegAddress regAddress, uint16_t & regData){
        i2c_drv_.readReg((uint8_t)regAddress, regData, MSB);
    }

    void writeReg(const RegAddress regAddress, const uint8_t regData){
        i2c_drv_.writeReg((uint8_t)regAddress, regData, MSB);
    }

    void readReg(const RegAddress regAddress, uint8_t & regData){
        i2c_drv_.readReg((uint8_t)regAddress, regData, MSB);
    }


    void requestPool(const RegAddress &regAddress, auto * datas, size_t len){
        i2c_drv_.readMulti((uint8_t)regAddress, datas, len, MSB);
    }

    real_t From12BitToGauss(const uint16_t data){
        real_t guass;
        s16_to_uni(data & 0x8fff, guass);
        guass *= lsb;
        return guass;
    }

    void setLsb(const Gain gain){
        switch(gain){
        case Gain::GL0_73:
            lsb = real_t(0.73);
            break;
        case Gain::GL0_92:
            lsb = real_t(0.92);
            break;
        case Gain::GL1_22:
            lsb  = real_t(1.22);
            break;
        case Gain::GL1_52:
            lsb = real_t(1.52);
            break;
        case Gain::GL2_27:
            lsb=real_t(2.27);
            break;
        case Gain::GL2_56:
            lsb = real_t(2.56);
            break;
        case Gain::GL3_03:
            lsb = real_t(3.03);
            break;
        case Gain::GL4_35:
            lsb = real_t(4.35);
            break;
        default:
            break;
        }
    }
public:
    HMC5883L(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    HMC5883L(I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    HMC5883L(I2c & bus):i2c_drv_(I2cDrv(bus, default_i2c_addr)){;}

    void init(){
        enableHighSpeed();
        enableContMode();
        setMeasurementMode(MeasurementMode::Norm);
        setDataRate(DataRate::DR75);
        setSampleNumber(SampleNumber::SN1);
        setGain(Gain::GL1_52);
    }
    void enableHighSpeed(const bool en = true){
        modeReg.hs = true;
        writeReg(RegAddress::Mode, modeReg);
    }

    void setMeasurementMode(const MeasurementMode mode){
        configAReg.measureMode = (uint8_t)mode;
        writeReg(RegAddress::ConfigA, configAReg);
    }

    void setDataRate(const DataRate rate){
        configAReg.dataRate = (uint8_t)rate;
        writeReg(RegAddress::ConfigA, configBReg);
    }

    void setSampleNumber(const SampleNumber number){
        configAReg.sampleNumber = (uint8_t)number;
        writeReg(RegAddress::ConfigA, configAReg);
    }

    void setGain(const Gain gain){
        configBReg.gain = (uint8_t)gain;
        writeReg(RegAddress::ConfigB, configBReg);
        setLsb(gain);
    }

    void setMode(const Mode mode){
        modeReg.mode = (uint8_t)mode;
        writeReg(RegAddress::Mode, modeReg);
    }

    std::tuple<real_t, real_t, real_t> getMagnet() override{
        real_t x = From12BitToGauss(magXReg);
        real_t y = From12BitToGauss(magYReg);
        real_t z = From12BitToGauss(magZReg);

        return std::make_tuple(x,y,z);
    }

    bool isChipValid(){
        uint8_t id[3] = {0};
        requestPool(RegAddress::IDA, id, 3);
        return (id[0] == 'H' && id[1] == '4' && id[2] == '3');
    }

    void update() override{
        requestPool(RegAddress::MagX, &magXReg, 3);
    }


    bool isIdle(){
        readReg(RegAddress::Status, statusReg);
        return statusReg.ready;
    }

    void enableContMode(const bool en = true){
        modeReg.mode = (uint8_t)(en ? Mode::Continuous : Mode::Single);
        writeReg(RegAddress::Mode, modeReg);
    }
};

};