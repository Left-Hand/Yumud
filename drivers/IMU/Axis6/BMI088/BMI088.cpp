#include "BMI088.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = ImuError;


template<typename Fn, typename Fn_Dur>
Result<void, Error> retry(const size_t times, Fn && fn, Fn_Dur && fn_dur){
    if constexpr(!std::is_null_pointer_v<Fn_Dur>) std::forward<Fn_Dur>(fn_dur)();
    Result<void, Error> res = std::forward<Fn>(fn)();
    if(!times) return res;
    else return retry(times - 1, std::forward<Fn>(fn), std::forward<Fn_Dur>(fn_dur));
}

template<typename Fn>
Result<void, Error> retry(const size_t times, Fn && fn){
    return retry(times, std::forward<Fn>(fn), nullptr);
}

static constexpr auto BMI088_TEMP_FACTOR = 0_r;
static constexpr auto BMI088_TEMP_OFFSET = 0_r;
static constexpr auto RETRY_TIMES = 3u;

static constexpr uint8_t ACC_CHIP_ID = 0;
static constexpr uint8_t GYR_CHIP_ID = 0;


Result<void, Error> BMI088_Acc::init(){
    TODO();
    return Ok();
}


Result<void, Error> BMI088_Acc::reset(){
    return phy_.write_command(0xb6);
}

Result<void, Error> BMI088_Acc::verify_chip_id(){
    return phy_.read_regs(acc_chipid_reg) 
        | rescond(acc_chipid_reg.data == ACC_CHIP_ID, Ok(), Err(Error(Error::WrongWhoAmI)));
}

Result<void, Error> BMI088_Acc::validate(){
    return reset()
        | phy_.validate()
        | retry(RETRY_TIMES, [&]{return verify_chip_id();}, [](){delay(1);})
        | set_acc_odr(AccOdr::_200Hz)
        | set_acc_bwp(AccBwp::Normal)
        | interrupts[0].enable_output(true)
        | interrupts[1].enable_output(true)
        // | retry(RETRY_TIMES, [&]{return verifyChipId();}, [](){delay(1);})
    ;
}

Result<void, Error> BMI088_Acc::update(){
    return phy_.read_burst(
        acc_x_reg.address, 
        &(acc_x_reg.as_ref()), 3
    );
}

Result<void, Error> BMI088_Gyr::update(){
    return phy_.read_burst(
        gyr_x_reg.address, 
        &(gyr_x_reg.as_ref()), 3
    );
}


Option<Vector3_t<q24>> BMI088_Acc::read_acc(){
    return Some(Vector3_t<q24>(
        acc_x_reg.as_val() * acc_scaler_,
        acc_y_reg.as_val() * acc_scaler_,
        acc_z_reg.as_val() * acc_scaler_
    ));
}
Option<real_t> BMI088_Acc::read_temp(){
	auto bmi088_raw_temp = int16_t((temp_reg.as_bytes()[0] << 3) | (temp_reg.as_bytes()[1] >> 5));
	if (bmi088_raw_temp > 1023) bmi088_raw_temp -= 2048;
    return Some(bmi088_raw_temp * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET);
}

Option<Vector3_t<q24>> BMI088_Gyr::read_gyr(){
    return Some(Vector3_t<q24>(
        gyr_x_reg.as_val() * gyr_scaler_,
        gyr_y_reg.as_val() * gyr_scaler_,
        gyr_z_reg.as_val() * gyr_scaler_
    ));
}

Result<void, Error> BMI088_Acc::set_acc_range(const AccRange range){
    acc_scaler_ = calculate_acc_scale(range).unwrap();
    acc_range_reg.acc_range = uint8_t(range);
    return phy_.write_regs(acc_range_reg);
}


Result<void, Error> BMI088_Acc::set_acc_bwp(const AccBwp bwp){
    acc_conf_reg.acc_bwp = uint8_t(bwp);
    return phy_.write_regs(acc_conf_reg);
}


Result<void, Error> BMI088_Acc::set_acc_odr(const AccOdr odr){
    acc_conf_reg.acc_odr = uint8_t(odr);
    return phy_.write_regs(acc_conf_reg);
}

Result<void, Error> BMI088_Gyr::set_gyr_range(const GyrRange range){
    gyr_scaler_ = calculate_gyr_scale(range).unwrap();
    gyro_range_reg.data = uint8_t(range);
    return phy_.write_regs(gyro_range_reg);
}

Result<void, Error> BMI088_Gyr::set_gyr_odr(const GyrOdr odr){
    gyro_bw_reg.data = uint8_t(odr);
    return phy_.write_regs(gyro_range_reg);
}


Result<void, Error> BMI088_Gyr::init(){
    TODO();
    return Ok();
}

Result<void, Error> BMI088_Gyr::reset(){
    TODO();
    return Ok();
}

Result<void, Error> BMI088_Gyr::validate(){
    TODO();
    return Ok();
}