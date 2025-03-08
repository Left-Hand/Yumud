#include "mpu6050.hpp"
#include "sys/debug/debug.hpp"

// #define MPU6050_DEBUG

#ifdef MPU6050_DEBUG
#undef MPU6050_DEBUG
#define MPU6050_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MPU6050_PANIC(...) PANIC{__VA_ARGS__}
#define MPU6050_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg) readReg(reg.address, reg).loc().expect();
#define WRITE_REG(reg) writeReg(reg.address, reg).loc().expect();
#else
#define MPU6050_DEBUG(...)
#define MPU6050_PANIC(...)  PANIC_NSRC()
#define MPU6050_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define READ_REG(reg) +readReg(reg.address, reg);
#define WRITE_REG(reg) +writeReg(reg.address, reg);
#endif


using namespace ymd;
using namespace ymd::drivers;

using DeviceResult = MPU6050::DeviceResult;

DeviceResult MPU6050::writeReg(const uint8_t addr, const uint8_t data){
    auto err = i2c_drv_.writeReg(uint8_t(addr), data);
    MPU6050_ASSERT(err.ok(), "MPU6050 write reg failed", err);
    return make_result(err);
}

DeviceResult MPU6050::readReg(const uint8_t addr, uint8_t & data){
    auto err = i2c_drv_.readReg(uint8_t(addr), data);
    MPU6050_ASSERT(err.ok(), "MPU6050 read reg failed", err);
    return make_result(err);
}

DeviceResult MPU6050::requestData(const uint8_t reg_addr, int16_t * datas, const size_t len){
    auto err = i2c_drv_.readMulti((uint8_t)reg_addr, datas, len, MSB);
    MPU6050_ASSERT(err.ok(), "MPU6050 read reg failed");
    return make_result(err);
}


bool MPU6050::verify(){
    //0x75 0x68
    uint8_t data = 0;
    auto err = readReg(0x75, data);
    return 
        MPU6050_ASSERT(err.is_ok(), "read who am I failed") and 
        MPU6050_ASSERT(data == 0x68, "who am I data wrong");
}


void MPU6050::init(){
    if(MPU6050_ASSERT(this->verify(), "MPU6050 verify failed")){
        +this->writeReg(0x6b, 0);
        +this->writeReg(0x19, 0x00);
        +this->writeReg(0x1a, 0x00);
        +this->writeReg(0x13, 0);
        +this->writeReg(0x15, 0);
        +this->writeReg(0x17, 0);
        +this->writeReg(0x38, 0x00);
        this->setAccRange(AccRange::_2G);
        this->setGyrRange(GyrRange::_1000deg);
    }
}

void MPU6050::update(){
    data_valid = this->requestData(RegAddress::AccX, &acc_x_reg, 7).is_ok();
}

std::tuple<real_t, real_t, real_t> MPU6050::getAcc(){
    real_t x = uni(acc_x_reg) * acc_scaler;
    real_t y = uni(acc_y_reg) * acc_scaler;
    real_t z = uni(acc_z_reg) * acc_scaler;
    return {x, y, z};
}

std::tuple<real_t, real_t, real_t> MPU6050::getGyr(){
    // if(!data_valid) return None;
    real_t x = uni(gyr_x_reg) * gyr_scaler;
    real_t y = uni(gyr_y_reg) * gyr_scaler;
    real_t z = uni(gyr_z_reg) * gyr_scaler;
    return {x, y, z};
}

Option<real_t> MPU6050::getTemperature(){
    return optcond(data_valid, Some(real_t(36.65f) + uni(temperature_reg) / 340));
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
