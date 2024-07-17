#ifndef __HMC5883L_HPP__

#define __HMC5883L_HPP__

#include "device_defs.h"
#include "../types/real.hpp"

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

    static constexpr uint8_t default_addr = 0x3d;
protected:
    I2cDrv bus_drv;

    real_t lsb;

    struct ConfigAReg{
        REG8_BEGIN
        uint8_t measureMode:3;
        uint8_t dataRate:2;
        uint8_t sampleNumber:2;
        uint8_t __resv__:1;
        REG8_END
    };

    struct ConfigBReg{
        REG8_BEGIN
        uint8_t __resv__:5;
        uint8_t gain:3;
        REG8_END
    };

    struct ModeReg{
        REG8_BEGIN
        uint8_t mode:2;
        uint8_t __resv__:5;
        uint8_t hs:1;
        REG8_END
    };

    struct StatusReg{
        REG8_BEGIN
        uint8_t ready:1;
        uint8_t lock:1;
        uint8_t __resv__:6;
        REG8_END
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
        bus_drv.writeReg((uint8_t)regAddress, regData);
    }

    void readReg(const RegAddress regAddress, uint16_t & regData){
        bus_drv.readReg((uint8_t)regAddress, regData);
    }

    void writeReg(const RegAddress regAddress, const uint8_t regData){
        bus_drv.writeReg((uint8_t)regAddress, regData);
    }

    void readReg(const RegAddress regAddress, uint8_t & regData){
        bus_drv.readReg((uint8_t)regAddress, regData);
    }


    void requestPool(const RegAddress &regAddress, auto * datas, size_t len){
        bus_drv.readPool((uint8_t)regAddress, datas, len);
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
    I2CDEV_CONTSRTUCTER(HMC5883L)

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
        writeReg(RegAddress::Mode, modeReg.data);
    }

    void setMeasurementMode(const MeasurementMode mode){
        configAReg.measureMode = (uint8_t)mode;
        writeReg(RegAddress::ConfigA, configAReg.data);
    }

    void setDataRate(const DataRate rate){
        configAReg.dataRate = (uint8_t)rate;
        writeReg(RegAddress::ConfigA, configBReg.data);
    }

    void setSampleNumber(const SampleNumber number){
        configAReg.sampleNumber = (uint8_t)number;
        writeReg(RegAddress::ConfigA, configAReg.data);
    }

    void setGain(const Gain gain){
        configBReg.gain = (uint8_t)gain;
        writeReg(RegAddress::ConfigB, configBReg.data);
        setLsb(gain);
    }

    void setMode(const Mode mode){
        modeReg.mode = (uint8_t)mode;
        writeReg(RegAddress::Mode, modeReg.data);
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
        readReg(RegAddress::Status, statusReg.data);
        return statusReg.ready;
    }

    void enableContMode(const bool en = true){
        modeReg.mode = (uint8_t)(en ? Mode::Continuous : Mode::Single);
        writeReg(RegAddress::Mode, modeReg.data);
    }
};

#endif