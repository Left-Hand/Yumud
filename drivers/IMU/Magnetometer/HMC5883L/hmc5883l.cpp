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


using Error = ImuError;

template<typename T = void>
using IResult= Result<T, Error>;

IResult<> HMC5883L::init(){
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = enable_high_speed(EN);
        res.is_err()) return res;
    if(const auto res = set_mode(Mode::Continuous);
        res.is_err()) return res;
    if(const auto res = set_odr(Odr::DR75);
        res.is_err()) return res;
    if(const auto res = set_sample_number(SampleNumber::SN1);
        res.is_err()) return res;
    if(const auto res = set_gain(Gain::GL1_52);
        res.is_err()) return res;
    return Ok();
}

IResult<> HMC5883L::enable_high_speed(const Enable en){
    auto reg = RegCopy(regs_.mode_reg);
    reg.hs = true;
    return write_reg(reg);
}


IResult<> HMC5883L::set_odr(const Odr rate){
    auto reg = RegCopy(regs_.config_a_reg);
    reg.dataRate = static_cast<uint8_t>(rate);
    return write_reg(reg);
}

IResult<> HMC5883L::set_sample_number(const SampleNumber number){
    auto reg = RegCopy(regs_.config_a_reg);
    reg.sampleNumber = static_cast<uint8_t>(number);
    return write_reg(reg);
}

IResult<> HMC5883L::set_gain(const Gain gain){
    auto reg = RegCopy(regs_.config_b_reg);
    reg.gain = gain;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    set_lsb(gain);
    return Ok();
}

IResult<> HMC5883L::set_mode(const Mode mode){
    auto reg = RegCopy(regs_.mode_reg);
    reg.mode = mode;
    return write_reg(reg);
}

IResult<math::Vec3<iq24>> HMC5883L::read_mag(){
    iq16 x = transform_raw_to_gauss(regs_.mag_x_reg, lsb_);
    iq16 y = transform_raw_to_gauss(regs_.mag_y_reg, lsb_);
    iq16 z = transform_raw_to_gauss(regs_.mag_z_reg, lsb_);

    return Ok(math::Vec3<iq24>(x,y,z));
}

IResult<> HMC5883L::validate(){
    if(const auto res = read_reg(regs_.id_a_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(regs_.id_b_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(regs_.id_c_reg);
        res.is_err()) return res;
    bool passed = (
        regs_.id_a_reg.to_bits() == 'H'
        and regs_.id_b_reg.to_bits() == '4' 
        and regs_.id_c_reg.to_bits() == '3');

    if(!passed) return Err(Error::InvalidChipId);
    return Ok();
}

IResult<> HMC5883L::update(){
    return read_burst(RegAddr::MagX, std::span(&regs_.mag_x_reg, 3));
}


IResult<bool> HMC5883L::is_data_ready(){
    auto & reg = regs_.status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.ready == false);
}
