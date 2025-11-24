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
static constexpr auto MAX_RETRY_TIMES = 3u;

static constexpr uint8_t GYR_CHIP_ID = 0;




IResult<> BMI088_Gyr::update(){
    auto & reg = regs_.gyr_x_reg;
    return phy_.read_burst(
        reg.address, 
        std::span(&(reg.as_bits_mut()), 3)
    );
}

IResult<Vec3<iq24>> BMI088_Gyr::read_gyr(){
    return Ok(Vec3<iq24>(
        regs_.gyr_x_reg.to_bits() * gyr_scaler_,
        regs_.gyr_y_reg.to_bits() * gyr_scaler_,
        regs_.gyr_z_reg.to_bits() * gyr_scaler_
    ));
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