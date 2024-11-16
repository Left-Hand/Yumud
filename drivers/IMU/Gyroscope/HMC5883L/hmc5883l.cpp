#include "hmc5883l.hpp"

using namespace ymd::drivers;


void HMC5883L::init(){
    enableHighSpeed();
    enableContMode();
    setMeasurementMode(MeasurementMode::Norm);
    setDataRate(DataRate::DR75);
    setSampleNumber(SampleNumber::SN1);
    setGain(Gain::GL1_52);
}

void HMC5883L::enableHighSpeed(const bool en){
    modeReg.hs = true;
    writeReg(RegAddress::Mode, modeReg);
}

void HMC5883L::setMeasurementMode(const MeasurementMode mode){
    configAReg.measureMode = (uint8_t)mode;
    writeReg(RegAddress::ConfigA, configAReg);
}

void HMC5883L::setDataRate(const DataRate rate){
    configAReg.dataRate = (uint8_t)rate;
    writeReg(RegAddress::ConfigA, configAReg);
}

void HMC5883L::setSampleNumber(const SampleNumber number){
    configAReg.sampleNumber = (uint8_t)number;
    writeReg(RegAddress::ConfigA, configAReg);
}

void HMC5883L::setGain(const Gain gain){
    configBReg.gain = (uint8_t)gain;
    writeReg(RegAddress::ConfigB, configBReg);
    setLsb(gain);
}

void HMC5883L::setMode(const Mode mode){
    modeReg.mode = (uint8_t)mode;
    writeReg(RegAddress::Mode, modeReg);
}

std::tuple<real_t, real_t, real_t> HMC5883L::getMagnet(){
    real_t x = From12BitToGauss(magXReg);
    real_t y = From12BitToGauss(magYReg);
    real_t z = From12BitToGauss(magZReg);

    return std::make_tuple(x,y,z);
}

bool HMC5883L::verify(){
    uint8_t id[3] = {0};
    requestPool(RegAddress::IDA, id, 3);
    return (id[0] == 'H' && id[1] == '4' && id[2] == '3');
}

void HMC5883L::update(){
    requestPool(RegAddress::MagX, &magXReg, 3);
}


bool HMC5883L::busy(){
    readReg(RegAddress::Status, statusReg);
    return statusReg.ready == false;
}

void HMC5883L::enableContMode(const bool en){
    modeReg.mode = (uint8_t)(en ? Mode::Continuous : Mode::Single);
    writeReg(RegAddress::Mode, modeReg);
}