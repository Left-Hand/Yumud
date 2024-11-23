#include "qmc5883l.hpp"

#define QMC5883L_DEBUG

#ifdef QMC5883L_DEBUG
#undef QMC5883L_DEBUG
#define QMC5883L_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define QMC5883L_PANIC(...) PANIC(__VA_ARGS__)
#define QMC5883L_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define QMC5883L_DEBUG(...)
#define QMC5883L_PANIC(...)  PANIC()
#define QMC5883L_ASSERT(cond, ...) ASSERT(cond)
#endif


using namespace ymd::drivers;

void QMC5883L::init(){
    if(this->verify()){
        this->setResetPeriod(1);
        this->enableContMode();
        this->setFullScale(FullScale::FS2G);
        this->setOverSampleRatio(OverSampleRatio::OSR512);
        this->setDataRate(DataRate::DR200);
    }
}

void QMC5883L::enableContMode(const bool en){
    configAReg.measureMode = (uint8_t)(en);
    writeReg(RegAddress::ConfigA, configAReg);
}

void QMC5883L::setDataRate(const DataRate rate){
    configAReg.dataRate = (uint8_t)rate;
    writeReg(RegAddress::ConfigA, configAReg);
}

void QMC5883L::setFullScale(const FullScale fullscale){
    configAReg.fullScale = (uint8_t)fullscale;
    writeReg(RegAddress::ConfigA, configAReg);
    setFs(fullscale);
}

void QMC5883L::setOverSampleRatio(const OverSampleRatio ratio){
    configAReg.OverSampleRatio = (uint8_t)ratio;
    writeReg(RegAddress::ConfigA, configAReg);
    setOvsfix(ratio);
}

void QMC5883L::update(){
    requestPool(RegAddress::MagX, &magXReg, 3);
}

std::tuple<real_t, real_t, real_t> QMC5883L::getMagnet(){
    return std::make_tuple(
        From16BitToGauss(magXReg),
        From16BitToGauss(magYReg),
        From16BitToGauss(magZReg)
    );
}

bool QMC5883L::verify(){
    readReg(RegAddress::ChipID, chipIDReg);
    return QMC5883L_ASSERT(chipIDReg == 0xFF, "QMC5883L not found");
}

void QMC5883L::setResetPeriod(const uint8_t resetPeriod){
    resetPeriodReg = resetPeriod;
    writeReg(RegAddress::ResetPeriod, resetPeriodReg);
}

void QMC5883L::reset(){
    configBReg.srst = true;
    writeReg(RegAddress::ConfigB, resetPeriodReg);
    configBReg.srst = false;
    writeReg(RegAddress::ConfigB, resetPeriodReg);
}

void QMC5883L::enableInterrupt(const bool en){
    configBReg.intEn = (uint8_t)(en);
    writeReg(RegAddress::ConfigB, configBReg);
}

bool QMC5883L::isOverflow(){
    readReg(RegAddress::Status, statusReg);
    return statusReg.ovl;
}