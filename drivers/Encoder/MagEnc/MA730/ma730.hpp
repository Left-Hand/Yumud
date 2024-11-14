#pragma once

#include "drivers/device_defs.h"
#include "drivers/Encoder/MagEncoder.hpp"

namespace ymd::drivers{

class MA730:public MagEncoder{
public:
    enum class Width:uint8_t{
        W90, W180, W270, W360
    };

    enum class Phase:uint8_t{
        P0, P90, P180, P270
    };

    enum class MagThreshold:uint8_t{
        mT23, mT38, mT53, mT67, mT81, mT95, mT109, mT123
    };

protected:
    SpiDrv spi_drv_;
    real_t lap_position;

    enum class RegAddress:uint8_t{
        ZeroDataLow,
        ZeroDataHigh,
        Trim,
        TrimConfig,
        ZParameters,
        PulsePerTurn,
        Threshold,
        Direction = 9,
        Magnitude = 27
    };


    struct TrimConfigReg:public Reg8{
        uint8_t enableX:1;
        uint8_t enableY:1;
        uint8_t :6;
    };

    struct ZParametersReg:public Reg8{
        uint8_t :2;
        uint8_t zPhase :2;
        uint8_t zWidth :2;
        uint8_t ppt:2;
    };

    struct ThresholdReg:public Reg8{
        uint8_t :2;
        uint8_t thresholdHigh :3;
        uint8_t thresholdLow :3;
    };

    struct DirectionReg:public Reg8{
        uint8_t :7;
        uint8_t direction :1;
    };

    struct MagnitudeReg:public Reg8{
        uint8_t :2;
        uint8_t mgl1:1;
        uint8_t mgl2:1;
        uint8_t :2;
        uint8_t magnitudeLow :1;
        uint8_t magnitudeHigh :1;
    };

    struct{
        uint16_t zeroDataReg;
        uint8_t trimReg;
        TrimConfigReg trimConfigReg;
        ZParametersReg zParametersReg;
        uint8_t pulsePerTurnReg;
        ThresholdReg thresholdReg;
        DirectionReg directionReg;
        MagnitudeReg magnitudeReg;
    };

    void writeReg(const RegAddress reg_addr, uint8_t data){
        spi_drv_.writeSingle((uint16_t)(0x8000 | ((uint8_t)reg_addr << 8) | data));
    }

    void readReg(const RegAddress reg_addr, uint8_t & reg){
        uint16_t dummy;
        spi_drv_.writeSingle((uint16_t)(0x4000 | ((uint8_t)reg_addr << 8)));
        spi_drv_.readSingle(dummy);
        reg = dummy >> 8;
    }

    void directRead(uint16_t & data){
        spi_drv_.readSingle(data);
    }

    uint16_t getRawData();

    void setZeroData(const uint16_t data);
public:
    MA730(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    MA730(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    MA730(Spi & _bus, const uint8_t index):spi_drv_(SpiDrv(_bus, index)){;}


    void init() override;
    void update();
    bool stable() override {return isMagnitudeProper();}


    void setZeroPosition(const real_t position);
    

    real_t getLapPosition() override{
        return lap_position;
    }

    void setTrimX(const real_t k);
    void setTrimY(const real_t k);
    void setTrim(const real_t am, const real_t e);

    void setMagThresholdLow(const MagThreshold threshold);
    void setMagThresholdHigh(const MagThreshold threshold);
    void setDirection(const bool direction);

    bool isMagnitudeLow();
    bool isMagnitudeHigh();
    bool isMagnitudeProper();

    void setZparameters(const Width width, const Phase phase);
    void setPulsePerTurn(const uint16_t _ppt);

};

class MA732:public MA730{
public:
    using MA730::MA730; 
};


};