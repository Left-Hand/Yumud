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
    if(const auto res = enable_high_speed();
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
    auto reg = RegCopy(mode_reg);
    reg.hs = true;
    return write_reg(reg);
}


IResult<> HMC5883L::set_odr(const Odr rate){
    auto reg = RegCopy(config_a_reg);
    reg.dataRate = static_cast<uint8_t>(rate);
    return write_reg(reg);
}

IResult<> HMC5883L::set_sample_number(const SampleNumber number){
    auto reg = RegCopy(config_a_reg);
    reg.sampleNumber = static_cast<uint8_t>(number);
    return write_reg(reg);
}

IResult<> HMC5883L::set_gain(const Gain gain){
    auto reg = RegCopy(config_b_reg);
    reg.gain = gain;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    set_lsb(gain);
    return Ok();
}

IResult<> HMC5883L::set_mode(const Mode mode){
    auto reg = RegCopy(mode_reg);
    reg.mode = mode;
    return write_reg(reg);
}

IResult<Vec3<q24>> HMC5883L::read_mag(){
    real_t x = transform_raw_to_gauss(magXReg, lsb_);
    real_t y = transform_raw_to_gauss(magYReg, lsb_);
    real_t z = transform_raw_to_gauss(magZReg, lsb_);

    return Ok(Vec3<q24>(x,y,z));
}

IResult<> HMC5883L::validate(){
    if(const auto res = read_reg(id_a_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(id_b_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(id_c_reg);
        res.is_err()) return res;
    bool passed = (
        id_a_reg.as_val() == 'H'
        and id_b_reg.as_val() == '4' 
        and id_c_reg.as_val() == '3');

    if(!passed) return Err(Error::WrongWhoAmI);
    return Ok();
}

IResult<> HMC5883L::update(){
    return read_burst(RegAddress::MagX, &magXReg, 3);
}


IResult<bool> HMC5883L::is_data_ready(){

    if(const auto res = read_reg(status_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(status_reg.ready == false);
}
