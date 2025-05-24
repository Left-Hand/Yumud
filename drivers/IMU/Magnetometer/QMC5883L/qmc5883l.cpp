#include "qmc5883l.hpp"
#include "core/debug/debug.hpp"

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


using namespace ymd;
using namespace ymd::drivers;

using Error = ImuError;

template<typename T = void>
using IResult= Result<T, Error>;


IResult<> QMC5883L::init(){
    if(const auto res = this->validate();
        res.is_err()) return res;
    if(const auto res = this->reset();
        res.is_err()) return res;
    if(const auto res = this->set_reset_period(1);
        res.is_err()) return res;
    if(const auto res = this->enable_cont_mode();
        res.is_err()) return res;
    if(const auto res = this->set_fs(FullScale::_2G);
        res.is_err()) return res;
    if(const auto res = this->set_over_sample_ratio(OverSampleRatio::_512);
        res.is_err()) return res;
    if(const auto res = this->set_odr(Odr::_200);
        res.is_err()) return res;

    return Ok();
}

IResult<> QMC5883L::enable_cont_mode(const bool en){
    configAReg.measureMode = en;
    return write_reg(RegAddress::ConfigA, configAReg);
}

IResult<> QMC5883L::set_odr(const Odr rate){
    configAReg.odr = rate;
    return write_reg(RegAddress::ConfigA, configAReg);
}

IResult<> QMC5883L::set_fs(const FullScale fullscale){
    configAReg.fs = fullscale;
    return write_reg(RegAddress::ConfigA, configAReg);
}

IResult<> QMC5883L::set_over_sample_ratio(const OverSampleRatio ratio){
    configAReg.ovs_ratio = ratio;
    return write_reg(RegAddress::ConfigA, configAReg);
}

IResult<> QMC5883L::update(){
    return read_burst(RegAddress::MagX, &magXReg, 3);
}

IResult<Vector3<q24>> QMC5883L::read_mag(){
    return Ok{Vector3<q24>{
        uni(int16_t(magXReg)) * scaler_.to_fullscale(),
        uni(int16_t(magYReg)) * scaler_.to_fullscale(),
        uni(int16_t(magZReg)) * scaler_.to_fullscale()
    }};
}

IResult<> QMC5883L::validate(){
    if(const auto res = read_reg(RegAddress::ChipID, chipIDReg);
        res.is_err()) return res;
    if(chipIDReg != 0xFF) return Err(Error::WrongWhoAmI);
    return Ok{};
}

IResult<> QMC5883L::set_reset_period(const uint8_t resetPeriod){
    resetPeriodReg = resetPeriod;
    return write_reg(RegAddress::ResetPeriod, resetPeriodReg);
}

IResult<> QMC5883L::reset(){
    configBReg.srst = true;
    if(const auto res = write_reg(RegAddress::ConfigB, configBReg);
        res.is_err()){
        configBReg.srst = false;
        return res;
    }
    configBReg.srst = false;
    return write_reg(RegAddress::ConfigB, configBReg);
}

IResult<> QMC5883L::enable_interrupt(const bool en){
    configBReg.intEn = en;
    return write_reg(RegAddress::ConfigB, configBReg);
}

IResult<bool> QMC5883L::is_overflow(){
    if(const auto res = read_reg(RegAddress::Status, statusReg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(statusReg.ovl));
}