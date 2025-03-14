#include "mpu6050.hpp"
#include "sys/debug/debug.hpp"

// #define MPU6050_DEBUG_EN

#ifdef MPU6050_DEBUG_EN
#define MPU6050_TODO(...) TODO()
#define MPU6050_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MPU6050_PANIC(...) PANIC{__VA_ARGS__}
#define MPU6050_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg) readReg(reg.address, reg).loc().expect();
#define WRITE_REG(reg) writeReg(reg.address, reg).loc().expect();
#else
#define MPU6050_DEBUG(...)
#define MPU6050_TODO(...) PANIC_NSRC()
#define MPU6050_PANIC(...)  PANIC_NSRC()
#define MPU6050_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define READ_REG(reg) !+readReg(reg.address, reg);
#define WRITE_REG(reg) !+writeReg(reg.address, reg);
#endif


using namespace ymd;
using namespace ymd::drivers;

using Error = MPU6050::Error;

Result<void, Error> MPU6050::writeReg(const uint8_t addr, const uint8_t data){
    if(p_i2c_drv_.has_value()){
        auto err = p_i2c_drv_->writeReg(uint8_t(addr), data);
        MPU6050_ASSERT(err.ok(), "MPU6050 write reg failed", err);
        return err;
    }else if(p_spi_drv_){
        MPU6050_TODO();
        __builtin_unreachable();
    }else{
        MPU6050_PANIC("no drv", p_i2c_drv_.has_value());
        __builtin_unreachable();
    }
}

Result<void, Error> MPU6050::readReg(const uint8_t addr, uint8_t & data){
    if(p_i2c_drv_.has_value()){
        auto err = p_i2c_drv_->readReg(uint8_t(addr), data);
        MPU6050_ASSERT(err.ok(), "MPU6050 read reg failed", err, addr);
        return err;
    }else if(p_spi_drv_){
        MPU6050_TODO();
        __builtin_unreachable();
    }else{
        MPU6050_PANIC("no drv", p_i2c_drv_.has_value());
        __builtin_unreachable();
    }
}

Result<void, Error> MPU6050::requestData(const uint8_t reg_addr, int16_t * datas, const size_t len){
    if(p_i2c_drv_.has_value()){
        auto err = p_i2c_drv_->readMulti((uint8_t)reg_addr, datas, len, MSB);
        MPU6050_ASSERT(err.ok(), "MPU6050 read reg failed");
        return err;
    }else if(p_spi_drv_){
        MPU6050_TODO();
        __builtin_unreachable();
    }else{
        MPU6050_PANIC("no drv");
        __builtin_unreachable();
    }
}

MPU6050::MPU6050(const hal::I2cDrv i2c_drv, const Package package):
    p_i2c_drv_(i2c_drv),
    package_(package){
    }
    
bool MPU6050::verify(){

    reset();
    const auto pkres = this->getPackage();
    if(!MPU6050_ASSERT(pkres.is_ok(), "read who am I failed")) return false;
    
    const auto package = pkres.unwrap();

    if(package != package_){
        switch(package){
            case Package::MPU6050: MPU6050_DEBUG("this is MPU6050 in fact"); break;
            case Package::MPU6500: MPU6050_DEBUG("this is MPU6500 in fact"); break;
            case Package::MPU9250: MPU6050_DEBUG("this is MPU9250 in fact"); break;
            default: MPU6050_PANIC("this is unknown device", uint8_t(package)); return false;
        }
        return true;
    }

    return true;
}


void MPU6050::init(){
    // this->setPackage(this->getPackage().);
    if(MPU6050_ASSERT(this->verify(), "MPU6050 verify failed")){
        !+this->writeReg(0x6b, 0);
        !+this->writeReg(0x19, 0x00);
        !+this->writeReg(0x1a, 0x00);
        !+this->writeReg(0x13, 0);
        !+this->writeReg(0x15, 0);
        !+this->writeReg(0x17, 0);
        !+this->writeReg(0x38, 0x00);
        this->setAccRange(AccRange::_2G);
        this->setGyrRange(GyrRange::_1000deg);
    }
}

void MPU6050::update(){
    data_valid = this->requestData(acc_x_reg.address, &acc_x_reg, 7).is_ok();
}

Option<Vector3r> MPU6050::getAcc(){
    real_t x = uni(acc_x_reg) * acc_scaler;
    real_t y = uni(acc_y_reg) * acc_scaler;
    real_t z = uni(acc_z_reg) * acc_scaler;
    return  Some{Vector3r{x, y, z}};
}

Option<Vector3r> MPU6050::getGyr(){
    // if(!data_valid) return None;
    real_t x = uni(gyr_x_reg) * gyr_scaler;
    real_t y = uni(gyr_y_reg) * gyr_scaler;
    real_t z = uni(gyr_z_reg) * gyr_scaler;
    return Some{Vector3r{x, y, z}};
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

Result<MPU6050::Package, Error> MPU6050::getPackage(){
    if(const auto err = readReg(whoami_reg.address, whoami_reg); err.is_err()){
        MPU6050_PANIC("read who am I failed");
    }
    return Ok{Package(whoami_reg.data)};
}

void MPU6050::setGyrRange(const GyrRange range){
    auto & reg = gyr_conf_reg;
    reg.fs_sel = uint8_t(range);
    WRITE_REG(reg);
    
    this->gyr_scaler = this->calculateGyrScale(range);
}


void MPU6050::reset(){
    if(p_i2c_drv_){
        p_i2c_drv_->release();
    }
}

void MPU6050::enableDirectMode(const Enable en){
    // int_pin_cfg_reg.bypass_en = bool(en);
    int_pin_cfg_reg.as_ref() = 0x22;
    WRITE_REG(int_pin_cfg_reg);
    !+writeReg(0x56, 0x01);
}