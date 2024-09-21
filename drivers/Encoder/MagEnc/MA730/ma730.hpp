#ifndef __MA730_HPP__
#define __MA730_HPP__

#include "../drivers/device_defs.h"
#include "../drivers/Encoder/MagEncoder.hpp"

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
    SpiDrv bus_drv;
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


    struct TrimConfigReg{
        uint8_t enableX:1;
        uint8_t enableY:1;
        uint8_t __resv__:6;
    };

    struct ZParametersReg{
        uint8_t __resv__ :2;
        uint8_t zPhase :2;
        uint8_t zWidth :2;
        uint8_t ppt:2;
    };

    struct ThresholdReg{
        uint8_t __resv__ :2;
        uint8_t thresholdHigh :3;
        uint8_t thresholdLow :3;
    };

    struct DirectionReg{
        uint8_t __resv__ :7;
        uint8_t direction :1;
    };

    struct MagnitudeReg{
        uint8_t __resv1__ :2;
        uint8_t mgl1:1;
        uint8_t mgl2:1;
        uint8_t __resv2__ :2;
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

    void writeReg(const RegAddress reg_addr, uint8_t reg_data){
        uint16_t ret;
        bus_drv.write((uint16_t)(0x8000 | ((uint8_t)reg_addr << 8) | *(uint8_t *)&reg_data));
        bus_drv.read(ret);
        *(uint8_t *)&reg_data = ret  >> 8;
    }

    void readReg(const RegAddress reg_addr, uint8_t & reg_data){
        uint16_t ret;
        bus_drv.write((uint16_t)(0x4000 | ((uint8_t)reg_addr << 8)));
        bus_drv.read(ret);
        *(uint8_t *)&reg_data = ret >> 8;
    }

    void directRead(uint16_t & data){
        bus_drv.read(data);
    }
public:
    MA730(const SpiDrv & _bus_drv):bus_drv(_bus_drv){;}
    MA730(SpiDrv && _bus_drv):bus_drv(_bus_drv){;}
    MA730(Spi & _bus, const uint8_t index):bus_drv(SpiDrv(_bus, index)){;}
    void init() override { }
    bool stable() override {return isMagnitudeProper();}

    uint16_t getRawData(){
        uint16_t data = 0;
        directRead(data);
        return data;
    }

    void setZeroData(const uint16_t data){
        zeroDataReg = data & 0xff;
        writeReg(RegAddress::ZeroDataLow, zeroDataReg & 0xff);
        writeReg(RegAddress::ZeroDataHigh, zeroDataReg >> 8);
    }

    void setZeroPosition(const real_t position){
        uint16_t data = 0;
        uni_to_u16(frac(position), data);
        setZeroData(data);
    }
    
    void update() override{
        uint16_t data = 0;
        directRead(data);
        u16_to_uni(data, lap_position);
    }

    real_t getLapPosition() override{
        return lap_position;
    }

    void setTrimX(const real_t k){
        trimReg= (uint8_t)((real_t(1) - real_t(1) / k) * 258);
        writeReg(RegAddress::Trim, trimReg);
        trimConfigReg.enableX = true;
        trimConfigReg.enableY = false;
        writeReg(RegAddress::TrimConfig, std::bit_cast<uint8_t>(trimConfigReg));
    }

    void setTrimY(const real_t k){
        trimReg = (uint8_t)((real_t(1) - k) * 258);
        writeReg(RegAddress::Trim, trimReg);
        trimConfigReg.enableX = false;
        trimConfigReg.enableY = true;
        writeReg(RegAddress::TrimConfig, std::bit_cast<uint8_t>(trimConfigReg));
    }
    void setTrim(const real_t am, const real_t e){
        real_t k = tan(am + e) / tan(am);
        if(k > real_t(1)) setTrimX(k);
        else setTrimY(k);
    }

    void setMagThresholdLow(const MagThreshold threshold){
        thresholdReg.thresholdLow = (uint8_t)threshold;
        writeReg(RegAddress::Threshold, std::bit_cast<uint8_t>(thresholdReg));
    }

    void setMagThresholdHigh(const MagThreshold threshold){
        thresholdReg.thresholdHigh = (uint8_t)threshold;
        writeReg(RegAddress::Threshold, std::bit_cast<uint8_t>(thresholdReg));
    }

    void setDirection(const bool direction){
        directionReg.direction = direction;
        writeReg(RegAddress::Direction, std::bit_cast<uint8_t>(directionReg));
    }

    bool isMagnitudeLow(){
        readReg(RegAddress::Magnitude, *reinterpret_cast<uint8_t *>(&magnitudeReg));
        bool correctMgl = !(magnitudeReg.mgl1 | magnitudeReg.mgl2);
        return correctMgl;
    }

    bool isMagnitudeHigh(){
        readReg(RegAddress::Magnitude, *reinterpret_cast<uint8_t *>(&magnitudeReg));
        return magnitudeReg.magnitudeHigh;
    }

    bool isMagnitudeProper(){
        readReg(RegAddress::Magnitude, *reinterpret_cast<uint8_t *>(&magnitudeReg));
        bool proper = !((!(magnitudeReg.mgl1 | magnitudeReg.mgl2)) | magnitudeReg.magnitudeHigh);
        return proper;
    }

    void setZparameters(const Width width, const Phase phase){
        zParametersReg.zWidth = (uint8_t)width;
        zParametersReg.zPhase = (uint8_t)phase;
        writeReg(RegAddress::ZParameters, std::bit_cast<uint8_t>(zParametersReg));
    }

    void setPulsePerTurn(const uint16_t _ppt){
        uint16_t ppt = CLAMP(_ppt - 1, 0, 1023);
        uint8_t ppt_l = ppt & 0b11;
        uint8_t ppt_h = ppt >> 2;
        zParametersReg.ppt = ppt_l;
        pulsePerTurnReg = ppt_h;
        writeReg(RegAddress::ZParameters, std::bit_cast<uint8_t>(zParametersReg));
        writeReg(RegAddress::PulsePerTurn, pulsePerTurnReg);
    }

};

#endif