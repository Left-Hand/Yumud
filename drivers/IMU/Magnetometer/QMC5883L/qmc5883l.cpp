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
    if(const auto res = this->enable_cont_mode(EN);
        res.is_err()) return res;
    if(const auto res = this->set_fs(FullScale::_2G);
        res.is_err()) return res;
    if(const auto res = this->set_over_sample_ratio(OverSampleRatio::_512);
        res.is_err()) return res;
    if(const auto res = this->set_odr(Odr::_200);
        res.is_err()) return res;

    return Ok();
}

IResult<> QMC5883L::enable_cont_mode(const Enable en){
    auto reg = RegCopy( config_a_reg);
    reg.measureMode = en == EN;
    return write_reg(reg);
}

IResult<> QMC5883L::set_odr(const Odr rate){
    auto reg = RegCopy( config_a_reg);
    reg.odr = rate;
    return write_reg(reg);
}

IResult<> QMC5883L::set_fs(const FullScale fullscale){
    auto reg = RegCopy( config_a_reg);
    reg.fs = fullscale;
    return write_reg(reg);
}

IResult<> QMC5883L::set_over_sample_ratio(const OverSampleRatio ratio){
    auto reg = RegCopy( config_a_reg);
    reg.ovs_ratio = ratio;
    return write_reg(reg);
}

IResult<> QMC5883L::update(){
    return read_burst(mag_x_reg.ADDRESS, std::span(&mag_x_reg.as_ref(), 3));
}

IResult<Vec3<iq24>> QMC5883L::read_mag(){
    return Ok{Vec3<iq24>{
        iq16::from_bits(mag_x_reg.as_val()) * scaler_.to_fullscale(),
        iq16::from_bits(mag_y_reg.as_val()) * scaler_.to_fullscale(),
        iq16::from_bits(mag_z_reg.as_val()) * scaler_.to_fullscale()
    }};
}

IResult<> QMC5883L::validate(){
    if(const auto res = read_reg(chip_id_reg);
        res.is_err()) return res;

    if(chip_id_reg.as_val() != 0xFF)
        return Err(Error::InvalidChipId);
    return Ok{};
}

IResult<> QMC5883L::set_reset_period(const uint8_t resetPeriod){
    auto reg =  RegCopy(reset_period_reg);
    reg.data = resetPeriod;
    return write_reg(reg);
}

IResult<> QMC5883L::reset(){
    auto reg = RegCopy(config_b_reg);
    reg.srst = true;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    reg.srst = false;
    return write_reg(reg);
}

IResult<> QMC5883L::enable_interrupt(const Enable en){
    auto reg = RegCopy(config_b_reg);
    reg.intEn = en == EN;
    return write_reg(reg);
}

IResult<bool> QMC5883L::is_overflow(){
    auto & reg = status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.ovl));
}