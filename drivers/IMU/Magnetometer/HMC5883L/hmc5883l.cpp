#include "hmc5883l.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

// #define HMC5883L_DEBUG

#ifdef HMC5883L_DEBUG
#undef HMC5883L_DEBUG
#define HMC5883L_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define HMC5883L_PANIC(...) PANIC(__VA_ARGS__)
#define HMC5883L_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define HMC5883L_DEBUG(...)
#define HMC5883L_PANIC(...)  PANIC{}
#define HMC5883L_ASSERT(cond, ...) ASSERT{cond}
#endif

void HMC5883L::init(){
    this->verify();
    this->enableHighSpeed();
    this->enableContMode();
    this->setMeasurementMode(MeasurementMode::Norm);
    this->setDataRate(DataRate::DR75);
    this->setSampleNumber(SampleNumber::SN1);
    this->setGain(Gain::GL1_52);
}

void HMC5883L::enableHighSpeed(const bool en){
    modeReg.hs = true;
    write_reg(RegAddress::Mode, modeReg);
}

void HMC5883L::setMeasurementMode(const MeasurementMode mode){
    configAReg.measureMode = (uint8_t)mode;
    write_reg(RegAddress::ConfigA, configAReg);
}

void HMC5883L::setDataRate(const DataRate rate){
    configAReg.dataRate = (uint8_t)rate;
    write_reg(RegAddress::ConfigA, configAReg);
}

void HMC5883L::setSampleNumber(const SampleNumber number){
    configAReg.sampleNumber = (uint8_t)number;
    write_reg(RegAddress::ConfigA, configAReg);
}

void HMC5883L::setGain(const Gain gain){
    configBReg.gain = (uint8_t)gain;
    write_reg(RegAddress::ConfigB, configBReg);
    setLsb(gain);
}

void HMC5883L::setMode(const Mode mode){
    modeReg.mode = (uint8_t)mode;
    write_reg(RegAddress::Mode, modeReg);
}

Option<Vector3_t<real_t>> HMC5883L::getMagnet(){
    real_t x = From12BitToGauss(magXReg);
    real_t y = From12BitToGauss(magYReg);
    real_t z = From12BitToGauss(magZReg);

    return Some(Vector3_t<real_t>(x,y,z));
}

bool HMC5883L::verify(){
    uint8_t id[3] = {0};
    read_reg(RegAddress::IDA, id[0]);
    read_reg(RegAddress::IDB, id[1]);
    read_reg(RegAddress::IDC, id[2]);
    bool passed = (id[0] == 'H' && id[1] == '4' && id[2] == '3');
    HMC5883L_ASSERT(passed, "HMC5883L not found!", id);
    return passed;
}

void HMC5883L::update(){
    read_burst(RegAddress::MagX, &magXReg, 3);
}


bool HMC5883L::busy(){
    read_reg(RegAddress::Status, statusReg);
    return statusReg.ready == false;
}

void HMC5883L::enableContMode(const bool en){
    modeReg.mode = (uint8_t)(en ? Mode::Continuous : Mode::Single);
    write_reg(RegAddress::Mode, modeReg);
}