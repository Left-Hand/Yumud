#include "AK8963.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Error = AK8963::Error;

// #define AK8963_DEBUG_EN
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
#define READ_REG(reg, ...) readReg(reg.address, reg).unwrap();
#define WRITE_REG(reg, ...) writeReg(reg.address, reg).unwrap();
#else
#define READ_REG(reg, ...) if(const auto res = readReg(reg.address, reg); res.is_err()) return res;
#define WRITE_REG(reg, ...) if(const auto res = writeReg(reg.address, reg); res.is_err()) return res;
#endif

#endif

Result<void, Error> AK8963::writeReg(const uint8_t addr, const uint8_t data){
    auto err = p_i2c_drv_->writeReg(uint8_t(addr), data);
    AK8963_ASSERT(err.ok(), "AK8963 write reg failed", err);
    return err;
}

Result<void, Error> AK8963::readReg(const uint8_t addr, uint8_t & data){
    auto err = p_i2c_drv_->readReg(uint8_t(addr), data);
    AK8963_ASSERT(err.ok(), "AK8963 read reg failed", err);
    return err;
}

Result<void, Error> AK8963::requestData(const uint8_t reg_addr, int16_t * datas, const size_t len){
    auto err = p_i2c_drv_->readBurst((uint8_t)reg_addr, std::span(datas, len), LSB);
    AK8963_ASSERT(err.ok(), "AK8963 read reg failed");
    return err;
}

Result<void, Error> AK8963::init(){
    AK8963_DEBUG("AK8963 init begin");
    AK8963_ASSERT(verify().is_ok(), "AK8963 verify failed");

    delay(2);
    reset();
    delay(2);

    {
        const auto coeff = getCoeff().unwrap();

        auto coeff2adj = [&](const uint8_t coeff) -> real_t{
            return ((iq_t<16>(coeff - 128) >> 8) + 1);
        };

        adj_scale = Vector3R(
            coeff2adj(coeff.x), coeff2adj(coeff.y), coeff2adj(coeff.z)
        );
    }

    delay(10);

    setMode(Mode::ContMode2).unwrap();
    setDataBits(16);

    AK8963_DEBUG("AK8963 init successfully!!");
    return Ok();
}


Result<void, Error> AK8963::verify(){
    p_i2c_drv_->release();

    if (!p_i2c_drv_->verify().ok()){
        AK8963_PANIC("AK8963 not founded");
        return Err{Error::DEVICE_NOT_FOUNDED};
    }

    readReg(wia_reg.address, wia_reg.data).unwrap();

    if (wia_reg.data != wia_reg.correct){
        if(wia_reg.data == 63){
            AK8963_DEBUG("it is normal to read 63 when comm speed too high");
        }
        AK8963_PANIC("AK8963 verify failed", wia_reg.data);
        return Err{Error::DEVICE_WHOAMI_FAILED};
    }
    return Ok();
}
Result<Vector3_t<uint8_t>, Error> AK8963::getCoeff(){
    // Vector3_t coeff = {};
    writeReg(0x0a, 0x0f).unwrap();
    READ_REG(asax_reg);
    READ_REG(asay_reg);
    READ_REG(asaz_reg);
    return Ok(Vector3_t<uint8_t>{asax_reg.data, asay_reg.data, asaz_reg.data});
}

Result<void, Error> AK8963::reset(){
    cntl2_reg.srst = 1;
    const auto res = writeReg(cntl2_reg.address, cntl2_reg);
    cntl2_reg.srst = 0;
    delay(1);
    return res;
}

Result<void, Error> AK8963::busy(){
    return Ok();
}

Result<void, Error> AK8963::stable(){return Ok();}


Result<void, Error> AK8963::disableI2c(){
    return Ok();
}


void AK8963::update(){
    // ak8963c-datasheet 8.3.5
    // when any of measurement data is read, be sure to read 
    // ST2 register at the end. 
    // auto lam2 = [&](){return readReg(st2_reg.address, st2_reg);};
    // using Fn = std::decay_t<decltype(lam2)>;
    // using Ret = function_traits<Fn>::return_type;
    // using t = std::invoke_result_t<decltype(lam2)>>;
    data_valid_ = this->requestData(mag_x_reg.address, &mag_x_reg, 3).is_ok();
    READ_REG(st2_reg);
    AK8963_ASSERT(!st2_reg.hofl, "data overflow");
    data_valid_ &= !st2_reg.hofl;
    // READ_REG(mag_y_reg);
    // READ_REG(mag_z_reg);
    // data_valid_ = true;
}
Option<Vector3R> AK8963::getMagnet(){
    return optcond(data_valid_, Vector3R{
        conv_data_to_ut(mag_x_reg.as_val(), data_is_16_bits_) * adj_scale.x,
        conv_data_to_ut(mag_y_reg.as_val(), data_is_16_bits_) * adj_scale.y,
        conv_data_to_ut(mag_z_reg.as_val(), data_is_16_bits_) * adj_scale.z}
    );
}

Result<void, Error> AK8963::setDataBits(const uint8_t bits){
    auto & reg = st2_reg;
    data_is_16_bits_ = [](const uint8_t bits_){
        switch(bits_){
            default: AK8963_PANIC(bits_, "bits is not allowed");
            case 14: return 0;
            case 16: return 1;
        }
    }(bits);

    reg.bitm = data_is_16_bits_;
    return writeReg(reg.address, reg);
}

Result<void, Error> AK8963::setMode(const AK8963::Mode mode){
    cntl1_reg.mode = uint8_t(mode);
    return writeReg(cntl1_reg.address, cntl1_reg);
}