#include "ma730.hpp"

void MA730::init(){
    
}

uint16_t MA730::getRawData(){
    uint16_t data = 0;
    directRead(data);
    return data;
}


void MA730::setZeroData(const uint16_t data){
    zeroDataReg = data & 0xff;
    writeReg(RegAddress::ZeroDataLow, zeroDataReg & 0xff);
    writeReg(RegAddress::ZeroDataHigh, zeroDataReg >> 8);
}


void MA730::setZeroPosition(const real_t position){
    uint16_t data = 0;
    uni_to_u16(frac(position), data);
    setZeroData(data);
}


void MA730::update(){
    uint16_t data = 0;
    directRead(data);
    u16_to_uni(data, lap_position);
}


void MA730::setTrimX(const real_t k){
    trimReg = (uint8_t)((real_t(1) - real_t(1) / k) * 258);
    writeReg(RegAddress::Trim, trimReg);
    trimConfigReg.enableX = true;
    trimConfigReg.enableY = false;
    writeReg(RegAddress::TrimConfig, uint8_t(trimConfigReg));
}

void MA730::setTrimY(const real_t k){
    trimReg = (uint8_t)((real_t(1) - k) * 258);
    writeReg(RegAddress::Trim, trimReg);
    trimConfigReg.enableX = false;
    trimConfigReg.enableY = true;
    writeReg(RegAddress::TrimConfig, uint8_t(trimConfigReg));
}


void MA730::setTrim(const real_t am, const real_t e){
    real_t k = tan(am + e) / tan(am);
    if(k > real_t(1)) setTrimX(k);
    else setTrimY(k);
}

void MA730::setMagThresholdLow(const MagThreshold threshold){
    thresholdReg.thresholdLow = (uint8_t)threshold;
    writeReg(RegAddress::Threshold, uint8_t(thresholdReg));
}

void MA730::setMagThresholdHigh(const MagThreshold threshold){
    thresholdReg.thresholdHigh = (uint8_t)threshold;
    writeReg(RegAddress::Threshold, uint8_t(thresholdReg));
}

void MA730::setDirection(const bool direction){
    directionReg.direction = direction;
    writeReg(RegAddress::Direction, uint8_t(directionReg));
}

bool MA730::isMagnitudeLow(){
    readReg(RegAddress::Magnitude, magnitudeReg);
    bool correctMgl = !(magnitudeReg.mgl1 | magnitudeReg.mgl2);
    return correctMgl;
}

bool MA730::isMagnitudeHigh(){
    readReg(RegAddress::Magnitude, magnitudeReg);
    return magnitudeReg.magnitudeHigh;
}

bool MA730::isMagnitudeProper(){
    readReg(RegAddress::Magnitude, magnitudeReg);
    bool proper = !((!(magnitudeReg.mgl1 | magnitudeReg.mgl2)) | magnitudeReg.magnitudeHigh);
    return proper;
}

void MA730::setZparameters(const Width width, const Phase phase){
    zParametersReg.zWidth = (uint8_t)width;
    zParametersReg.zPhase = (uint8_t)phase;
    writeReg(RegAddress::ZParameters, uint8_t(zParametersReg));
}

void MA730::setPulsePerTurn(const uint16_t _ppt){
    uint16_t ppt = CLAMP(_ppt - 1, 0, 1023);
    uint8_t ppt_l = ppt & 0b11;
    uint8_t ppt_h = ppt >> 2;
    
    zParametersReg.ppt = ppt_l;
    pulsePerTurnReg = ppt_h;

    writeReg(RegAddress::ZParameters, uint8_t(zParametersReg));
    writeReg(RegAddress::PulsePerTurn, pulsePerTurnReg);
}
