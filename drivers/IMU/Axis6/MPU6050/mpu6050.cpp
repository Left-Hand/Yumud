#include "mpu6050.hpp"


#define MPU6050_DEBUG

#ifdef MPU6050_DEBUG
#undef MPU6050_DEBUG
#define MPU6050_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MPU6050_PANIC(...) PANIC(__VA_ARGS__)
#define MPU6050_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define MPU6050_DEBUG(...)
#define MPU6050_PANIC(...)  PANIC()
#define MPU6050_ASSERT(cond, ...) ASSERT(cond)
#endif


#define WRITE_REG(reg) this->writeReg(reg.address, reg);
#define READ_REG(reg) this->readReg(reg.address, reg);

using namespace ymd::drivers;
void MPU6050::init(){
    bool ok = MPU6050_ASSERT(this->verify(), "MPU6050 verify failed")
    if(ok){
        this->writeReg(0x6b, 0);
        this->writeReg(0x19, 0x00);
        this->writeReg(0x1a, 0x00);
        this->writeReg(0x13, 0);
        this->writeReg(0x15, 0);
        this->writeReg(0x17, 0);
        this->writeReg(0x38, 0x00);
        this->setAccRange(AccRange::_2G);
        this->setGyrRange(GyrRange::_1000deg);
    }
}

void MPU6050::update(){
    this->requestData(RegAddress::AccX, &acc_x_reg, 7);
}

std::tuple<real_t, real_t, real_t> MPU6050::getAcc(){
    real_t x = uni(acc_x_reg) * acc_scaler;
    real_t y = uni(acc_y_reg) * acc_scaler;
    real_t z = uni(acc_z_reg) * acc_scaler;
    return {x, y, z};
}

std::tuple<real_t, real_t, real_t> MPU6050::getGyr(){
    real_t x = uni(gyr_x_reg) * gyr_scaler;
    real_t y = uni(gyr_y_reg) * gyr_scaler;
    real_t z = uni(gyr_z_reg) * gyr_scaler;
    return {x, y, z};
}

real_t MPU6050::getTemperature(){
    return real_t(36.65f) + uni(temperature_reg) / 340;
}


bool MPU6050::verify(){
    //0x75 0x68
    uint8_t data;
    readReg(0x75, data);
    return data == 0x68;
}

void MPU6050::setAccRange(const AccRange range){
    auto & reg = acc_conf_reg;
    reg.afs_sel = uint8_t(range);
    WRITE_REG(reg);
    
    this->acc_scaler = this->calculateAccScale(range);
}

void MPU6050::setGyrRange(const GyrRange range){
    auto & reg = gyr_conf_reg;
    reg.fs_sel = uint8_t(range);
    WRITE_REG(reg);
    
    this->gyr_scaler = this->calculateGyrScale(range);
}

real_t MPU6050::calculateAccScale(const AccRange range){
    scexpr double g = 9.806;
    switch(range){
        default:
        case AccRange::_2G:
            return real_t(g * 2);
        case AccRange::_4G:
            return real_t(g * 4);
        case AccRange::_8G:
            return real_t(g * 8);
        case AccRange::_16G:
            return real_t(g * 16);
    }
}

real_t MPU6050::calculateGyrScale(const GyrRange range){
    switch(range){
        default:
        case GyrRange::_250deg:
            return real_t(ANGLE2RAD(250));
        case GyrRange::_500deg:
            return real_t(ANGLE2RAD(500));
        case GyrRange::_1000deg:
            return real_t(ANGLE2RAD(1000));
        case GyrRange::_2000deg:
            return real_t(ANGLE2RAD(2000));
    }
}

