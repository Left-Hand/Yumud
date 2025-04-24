#include "ma730.hpp"
#include "core/math/realmath.hpp"

using namespace ymd::drivers;
using namespace ymd;

#define MA730_DEBUG

#ifdef MA730_DEBUG
#undef MA730_DEBUG
#define MA730_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MA730_PANIC(...) PANIC(__VA_ARGS__)
#define MA730_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define MA730_DEBUG(...)
#define MA730_PANIC(...)  PANIC()
#define MA730_ASSERT(cond, ...) ASSERT(cond)
#endif

void MA730::init(){
    get_lap_position();
}

BusError MA730::write_reg(const RegAddress reg_addr, uint8_t data){
    const auto err = spi_drv_.write_single((uint16_t)(0x8000 | ((uint8_t)reg_addr << 8) | data));
    return err;
}

BusError MA730::read_reg(const RegAddress reg_addr, uint8_t & reg){
    uint16_t dummy;
    spi_drv_.write_single((uint16_t)(0x4000 | ((uint8_t)reg_addr << 8))).unwrap();
    const auto err = spi_drv_.read_single(dummy);
    reg = dummy >> 8;
    // ASSERT(BusError::OK);
    // PANIC("???");
    return err;
}

BusError MA730::direct_read(uint16_t & data){
    const auto err = spi_drv_.read_single(data);
    // ASSERT(BusError::OK);
    return err;
}


uint16_t MA730::get_raw_data(){
    uint16_t data = 0;
    direct_read(data);
    return data;
}


void MA730::set_zero_data(const uint16_t data){
    zeroDataReg = data & 0xff;
    write_reg(RegAddress::ZeroDataLow, zeroDataReg & 0xff);
    write_reg(RegAddress::ZeroDataHigh, zeroDataReg >> 8);
}


void MA730::set_zero_position(const real_t position){
    auto data = uni_to_u16(frac(position));
    set_zero_data(data);
}


void MA730::update(){
    uint16_t data = 0;
    direct_read(data);
    lap_position = u16_to_uni(data);
}


void MA730::set_trim_x(const real_t k){
    trimReg = (uint8_t)((real_t(1) - real_t(1) / k) * 258);
    write_reg(RegAddress::Trim, trimReg);
    trimConfigReg.enableX = true;
    trimConfigReg.enableY = false;
    write_reg(RegAddress::TrimConfig, uint8_t(trimConfigReg));
}

void MA730::set_trim_y(const real_t k){
    trimReg = (uint8_t)((real_t(1) - k) * 258);
    write_reg(RegAddress::Trim, trimReg);
    trimConfigReg.enableX = false;
    trimConfigReg.enableY = true;
    write_reg(RegAddress::TrimConfig, uint8_t(trimConfigReg));
}


void MA730::set_trim(const real_t am, const real_t e){
    real_t k = std::tan(am + e) / std::tan(am);
    if(k > real_t(1)) set_trim_x(k);
    else set_trim_y(k);
}

void MA730::set_mag_threshold_low(const MagThreshold threshold){
    thresholdReg.thresholdLow = (uint8_t)threshold;
    write_reg(RegAddress::Threshold, uint8_t(thresholdReg));
}

void MA730::set_mag_threshold_high(const MagThreshold threshold){
    thresholdReg.thresholdHigh = (uint8_t)threshold;
    write_reg(RegAddress::Threshold, uint8_t(thresholdReg));
}

void MA730::set_direction(const bool direction){
    directionReg.direction = direction;
    write_reg(RegAddress::Direction, uint8_t(directionReg));
}

bool MA730::is_magnitude_low(){
    read_reg(RegAddress::Magnitude, magnitudeReg);
    bool correctMgl = !(magnitudeReg.mgl1 | magnitudeReg.mgl2);
    return correctMgl;
}

bool MA730::is_magnitude_high(){
    read_reg(RegAddress::Magnitude, magnitudeReg);
    return magnitudeReg.magnitudeHigh;
}

bool MA730::is_magnitude_proper(){
    read_reg(RegAddress::Magnitude, magnitudeReg);
    bool proper = !((!(magnitudeReg.mgl1 | magnitudeReg.mgl2)) | magnitudeReg.magnitudeHigh);
    return proper;
}

void MA730::set_zparameters(const Width width, const Phase phase){
    zParametersReg.zWidth = (uint8_t)width;
    zParametersReg.zPhase = (uint8_t)phase;
    write_reg(RegAddress::ZParameters, uint8_t(zParametersReg));
}

void MA730::set_pulse_per_turn(const uint16_t _ppt){
    uint16_t ppt = CLAMP(_ppt - 1, 0, 1023);
    uint8_t ppt_l = ppt & 0b11;
    uint8_t ppt_h = ppt >> 2;
    
    zParametersReg.ppt = ppt_l;
    pulsePerTurnReg = ppt_h;

    write_reg(RegAddress::ZParameters, uint8_t(zParametersReg));
    write_reg(RegAddress::PulsePerTurn, pulsePerTurnReg);
}
