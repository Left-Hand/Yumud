#include "BMI088.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = BoschSensor::Error;


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
    
    return Ok();
}


Result<void, Error> BMI088_Acc::reset(){
    return write_command(0xb6);
}

Result<void, Error> BMI088_Acc::verifyChipId(){
    auto err = read_regs(acc_chipid_reg);
    return err | rescond(acc_chipid_reg.data == ACC_CHIP_ID, Ok(), Err(Error::UNSPECIFIED));
}

Result<void, Error> BMI088_Acc::verify(){
    return reset()
        | Result<void, Error>(i2c_drv_->verify())
        | retry(RETRY_TIMES, [&]{return verifyChipId();}, [](){delay(1);})
        | setAccOdr(AccOdr::_200Hz)
        | setAccBwp(AccBwp::Normal)
        | interrupts[0].enableOutput(true)
        | interrupts[1].enableOutput(true)
        // | retry(RETRY_TIMES, [&]{return verifyChipId();}, [](){delay(1);})
    ;
}

Result<void, Error> BMI088_Acc::update(){
    return read_burst(acc_x_reg.address, 
        // std::span(&(acc_x_reg.as_ref()), 3),
        // LSB
        &(acc_x_reg.as_ref()), 3
    );
}

Result<void, Error> BMI088_Gyr::update(){
    return read_burst(gyr_x_reg.address, 
        // std::span(&(gyr_x_reg.as_ref()), 3),
        &(gyr_x_reg.as_ref()), 3
    );
}


Option<Vector3R> BMI088_Acc::getAcc(){
    return Some(Vector3R(
        acc_x_reg.as_val() * acc_scaler_,
        acc_y_reg.as_val() * acc_scaler_,
        acc_z_reg.as_val() * acc_scaler_
    ));
}
Option<real_t> BMI088_Acc::getTemperature(){
	auto bmi088_raw_temp = int16_t((temp_reg.as_bytes()[0] << 3) | (temp_reg.as_bytes()[1] >> 5));
	if (bmi088_raw_temp > 1023) bmi088_raw_temp -= 2048;
    return Some(bmi088_raw_temp * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET);
}

Option<Vector3R> BMI088_Gyr::getGyr(){
    return Some(Vector3R(
        gyr_x_reg.as_val() * gyr_scaler_,
        gyr_y_reg.as_val() * gyr_scaler_,
        gyr_z_reg.as_val() * gyr_scaler_
    ));
}

Result<void, Error> BMI088_Acc::setAccRange(const AccRange range){
    acc_scaler_ = calculateAccScale(range).unwrap();
    acc_range_reg.acc_range = uint8_t(range);
    return write_regs(acc_range_reg);
}


Result<void, Error> BMI088_Acc::setAccBwp(const AccBwp bwp){
    acc_conf_reg.acc_bwp = uint8_t(bwp);
    return write_regs(acc_conf_reg);
}


Result<void, Error> BMI088_Acc::setAccOdr(const AccOdr odr){
    acc_conf_reg.acc_odr = uint8_t(odr);
    return write_regs(acc_conf_reg);
}

Result<void, Error> BMI088_Gyr::setGyrRange(const GyrRange range){
    gyr_scaler_ = calculateGyrScale(range).unwrap();
    gyro_range_reg.data = uint8_t(range);
    return write_regs(gyro_range_reg);
}

Result<void, Error> BMI088_Gyr::setGyrOdr(const GyrOdr odr){
    gyro_bw_reg.data = uint8_t(odr);
    return write_regs(gyro_range_reg);
}