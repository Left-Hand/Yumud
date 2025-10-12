#include "BMI088.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = ImuError;

template<typename T = void>
using IResult = Result<T, Error>;

template<typename Fn, typename Fn_Dur>
IResult<> retry(const size_t times, Fn && fn, Fn_Dur && fn_dur){
    if constexpr(!std::is_null_pointer_v<Fn_Dur>) std::forward<Fn_Dur>(fn_dur)();
    IResult<> res = std::forward<Fn>(fn)();
    if(!times) return res;
    else return retry(times - 1, std::forward<Fn>(fn), std::forward<Fn_Dur>(fn_dur));
}

template<typename Fn>
IResult<> retry(const size_t times, Fn && fn){
    return retry(times, std::forward<Fn>(fn), nullptr);
}

static constexpr auto BMI088_TEMP_FACTOR = 0_r;
static constexpr auto BMI088_TEMP_OFFSET = 0_r;
static constexpr auto RETRY_TIMES = 3u;

static constexpr uint8_t ACC_CHIP_ID = 0;
static constexpr uint8_t GYR_CHIP_ID = 0;


IResult<> BMI088_Acc::init(){
    TODO();
    return Ok();
}


IResult<> BMI088_Acc::reset(){
    return phy_.write_command(0xb6);
}

IResult<> BMI088_Acc::verify_chip_id(){
    auto & reg = regs_.acc_chipid_reg;
    if(const auto res = phy_.read_regs(reg);
        res.is_err()) return res;

    if(reg.data != ACC_CHIP_ID)
        Err(Error::WrongWhoAmI);

    return Ok();
}

IResult<> BMI088_Acc::validate(){
    if(const auto res = reset();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = phy_.validate();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = retry(RETRY_TIMES, [&]{return verify_chip_id();}, [](){clock::delay(1ms);});
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_acc_odr(AccOdr::_200Hz);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_acc_bwp(AccBwp::Normal);
        res.is_err()) return Err(res.unwrap_err());
        // | interrupts[0].enable_output(EN)
        // | interrupts[1].enable_output(EN)
        // | retry(RETRY_TIMES, [&]{return verifyChipId();}, [](){clock::delay(1ms);})

    return Ok();
}

IResult<> BMI088_Acc::update(){
    auto & reg = regs_.acc_x_reg;
    return phy_.read_burst(
        reg.address, 
        std::span(&(reg.as_ref()), 3)
    );
}

IResult<> BMI088_Gyr::update(){
    auto & reg = regs_.gyr_x_reg;
    return phy_.read_burst(
        reg.address, 
        std::span(&(reg.as_ref()), 3)
    );
}


IResult<Vec3<q24>> BMI088_Acc::read_acc(){
    return Ok(Vec3<q24>(
        regs_.acc_x_reg.as_val() * acc_scaler_,
        regs_.acc_y_reg.as_val() * acc_scaler_,
        regs_.acc_z_reg.as_val() * acc_scaler_
    ));
}
IResult<q24> BMI088_Acc::read_temp(){
    auto & reg = regs_.temp_reg;
    auto bytes = reg.as_bytes();
	auto bmi088_raw_temp = int16_t((bytes[0] << 3) | (bytes[1] >> 5));
	if (bmi088_raw_temp > 1023) bmi088_raw_temp -= 2048;
    return Ok(bmi088_raw_temp * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET);
}

IResult<Vec3<q24>> BMI088_Gyr::read_gyr(){
    return Ok(Vec3<q24>(
        regs_.gyr_x_reg.as_val() * gyr_scaler_,
        regs_.gyr_y_reg.as_val() * gyr_scaler_,
        regs_.gyr_z_reg.as_val() * gyr_scaler_
    ));
}

IResult<> BMI088_Acc::set_acc_fs(const AccFs fs){
    auto & reg = regs_.acc_range_reg;
    acc_scaler_ = calculate_acc_scale(fs);
    reg.acc_range = uint8_t(fs);
    return phy_.write_regs(reg);
}


IResult<> BMI088_Acc::set_acc_bwp(const AccBwp bwp){
    auto & reg = regs_.acc_conf_reg;
    reg.acc_bwp = uint8_t(bwp);
    return phy_.write_regs(reg);
}


IResult<> BMI088_Acc::set_acc_odr(const AccOdr odr){
    auto & reg = regs_.acc_conf_reg;
    reg.acc_odr = uint8_t(odr);
    return phy_.write_regs(reg);
}

IResult<> BMI088_Gyr::set_gyr_fs(const GyrFs fs){
    gyr_scaler_ = calculate_gyr_scale(fs);
    auto & reg = regs_.gyro_range_reg;
    reg.data = uint8_t(fs);
    return phy_.write_regs(reg);
}

IResult<> BMI088_Gyr::set_gyr_odr(const GyrOdr odr){
    auto & reg = regs_.gyro_bw_reg;
    reg.data = uint8_t(odr);
    return phy_.write_regs(reg);
}


IResult<> BMI088_Gyr::init(){
    TODO();
    return Ok();
}

IResult<> BMI088_Gyr::reset(){
    TODO();
    return Ok();
}

IResult<> BMI088_Gyr::validate(){
    TODO();
    return Ok();
}