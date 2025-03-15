#include "BMI088.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = BoschSensor::Error;

Result<void, Error> BMI088_Acc::init(){
    return Ok();
}

Result<void, Error> BMI088_Acc::update(){
    return Ok();
}


Option<Vector3R> BMI088_Acc::getAcc(){
    return Some(Vector3R(0,0,0));
}
Option<real_t> BMI088_Acc::getTemperature(){
    return Some(0_r);
}

Result<void, Error> BMI088_Acc::setAccRange(const GyrRange range){
    acc_range_reg.acc_range = uint8_t(range);
    return writeRegs(acc_range_reg);
}


Result<void, Error> BMI088_Acc::setAccBwp(const AccBwp bwp){
    acc_conf_reg.acc_bwp = uint8_t(bwp);
    return writeRegs(acc_conf_reg);
}


Result<void, Error> BMI088_Acc::setAccOdr(const AccOdr odr){
    acc_conf_reg.acc_odr = uint8_t(odr);
    return writeRegs(acc_conf_reg);
}