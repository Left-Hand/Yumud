#include "AK8963.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Error = AK8963::Error;

#define AK8963_DEBUG_EN
#define AK8963_NOTHROW_EN

#ifdef AK8963_DEBUG_EN
#define AK8963_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define AK8963_PANIC(...) PANIC{__VA_ARGS__}
#define AK8963_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg, ...) readReg(reg.address, reg).loc().expect(__VA_ARGS__);
#define WRITE_REG(reg, ...) writeReg(reg.address, reg).loc().expect(__VA_ARGS__);
#else
#define AK8963_DEBUG(...)
#define AK8963_PANIC(...)  PANIC_NSRC()
#define AK8963_ASSERT(cond, ...) ASSERT_NSRC(cond)

#ifdef AK8963_NOTHROW_EN
#define READ_REG(reg, ...) !+readReg(reg.address, reg);
#define WRITE_REG(reg, ...) !+writeReg(reg.address, reg);
#else
#define READ_REG(reg, ...) if(const auto res = readReg(reg.address, reg); res.is_err()) return res;
#define WRITE_REG(reg, ...) if(const auto res = writeReg(reg.address, reg); res.is_err()) return res;
#endif

#endif

Result<void, Error> AK8963::writeReg(const uint8_t addr, const uint8_t data){
    auto err = i2c_drv_->writeReg(uint8_t(addr), data);
    AK8963_ASSERT(err.ok(), "AK8963 write reg failed", err);
    return err;
}

Result<void, Error> AK8963::readReg(const uint8_t addr, uint8_t & data){
    auto err = i2c_drv_->readReg(uint8_t(addr), data);
    AK8963_ASSERT(err.ok(), "AK8963 read reg failed", err);
    return err;
}

Result<void, Error> AK8963::requestData(const uint8_t reg_addr, int16_t * datas, const size_t len){
    auto err = i2c_drv_->readMulti((uint8_t)reg_addr, datas, len, MSB);
    AK8963_ASSERT(err.ok(), "AK8963 read reg failed");
    return err;
}

Result<void, Error> AK8963::init(){
    return Ok();
}

void AK8963::update(){

}

Result<void, Error> AK8963::verify(){
    return Ok();
}

Result<void, Error> AK8963::reset(){
    return Ok();
}

Result<void, Error> AK8963::busy(){
    return Ok();
}

Result<void, Error> AK8963::stable(){return Ok();}

Result<void, Error> AK8963::setMode(const Mode mode){
    return Ok();
}

Result<void, Error> AK8963::disableI2c(){
    return Ok();
}

Option<Vector3> AK8963::getMagnet(){
    return Some{Vector3{0,0,0}};
}