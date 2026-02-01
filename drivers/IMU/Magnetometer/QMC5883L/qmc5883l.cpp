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
    auto reg = RegCopy(regs_.config_a_reg);
    reg.measure_mode = en == EN;
    return write_reg(reg);
}

IResult<> QMC5883L::set_odr(const Odr rate){
    auto reg = RegCopy(regs_.config_a_reg);
    reg.odr = rate;
    return write_reg(reg);
}

IResult<> QMC5883L::set_fs(const FullScale fs){
    auto reg = RegCopy(regs_.config_a_reg);
    reg.fs = fs;
    return write_reg(reg);
}

IResult<> QMC5883L::set_over_sample_ratio(const OverSampleRatio ratio){
    auto reg = RegCopy(regs_.config_a_reg);
    reg.ovs_ratio = ratio;
    return write_reg(reg);
}

IResult<> QMC5883L::update(){
    std::array<int16_t, 3> buf;
    if(const auto res = read_burst(regs_.mag_x_reg.ADDRESS, std::span(buf));
        res.is_err()) return res;

    regs_.mag_x_reg.bits = buf[0];
    regs_.mag_y_reg.bits = buf[1];
    regs_.mag_z_reg.bits = buf[2];

    return Ok();
}

IResult<math::Vec3<iq16>> QMC5883L::read_mag(){
    return Ok{math::Vec3<iq16>{
        iq16::from_bits(std::bit_cast<int16_t>(regs_.mag_x_reg.to_bits())) * scaler_,
        iq16::from_bits(std::bit_cast<int16_t>(regs_.mag_y_reg.to_bits())) * scaler_,
        iq16::from_bits(std::bit_cast<int16_t>(regs_.mag_z_reg.to_bits())) * scaler_
    }};
}

IResult<> QMC5883L::validate(){
    if(const auto res = read_reg(regs_.chip_id_reg);
        res.is_err()) return res;

    if(regs_.chip_id_reg.to_bits() != 0xFF)
        return Err(Error::InvalidChipId);
    return Ok{};
}

IResult<> QMC5883L::set_reset_period(const uint8_t reset_period){
    auto reg =  RegCopy(regs_.reset_period_reg);
    reg.bits = reset_period;
    return write_reg(reg);
}

IResult<> QMC5883L::reset(){
    auto reg = RegCopy(regs_.config_b_reg);
    reg.soft_reset = true;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    reg.soft_reset = false;
    return write_reg(reg);
}

IResult<> QMC5883L::enable_interrupt(const Enable en){
    auto reg = RegCopy(regs_.config_b_reg);
    reg.int_en = en == EN;
    return write_reg(reg);
}

IResult<bool> QMC5883L::is_overflow(){
    auto & reg = regs_.status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.ovl));
}

IResult<bool> QMC5883L::is_busy(){
    auto & reg = regs_.status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.ready == false);
}