#include "Estimator.hpp"

using namespace ymd::drivers;
using namespace gxm;

void Estimator::update(){
    acc_gyro_sensor_.update();
    mag_sensor_.update();

    acc3_ = bias_.acc.xform(Vector3(acc_gyro_sensor_.getAccel()));
    gyro3_ = Vector3(acc_gyro_sensor_.getGyro()) - bias_.gyro;
    mag3_ = bias_.mag.xform(Vector3(mag_sensor_.getMagnet()));
}

Quat Estimator::calculateAccelBias(){
    Vector3 acc_bias_sum = Vector3::ZERO;
    
    for(size_t i = 0; i < config_.calibrate_times; i++){
        acc_gyro_sensor_.update();
        acc_bias_sum += Vector3(acc_gyro_sensor_.getAccel());
    }

    const auto acc_vec3 = acc_bias_sum / config_.calibrate_times;
    return Quat(Vector3(0,0,-1), acc_vec3.normalized()).inverse();
}

Vector3 Estimator::calculateGyroBias(){
    Vector3 gyro_bias_sum = Vector3::ZERO;
    
    for(size_t i = 0; i < config_.calibrate_times; i++){
        acc_gyro_sensor_.update();
        gyro_bias_sum += Vector3(acc_gyro_sensor_.getGyro());
    }

    return gyro_bias_sum / config_.calibrate_times;
}


Quat Estimator::calculateMagBias(){
    Vector3 mag_bias_sum = Vector3::ZERO;
    
    for(size_t i = 0; i < config_.calibrate_times; i++){
        mag_sensor_.update();
        mag_bias_sum += Vector3(mag_sensor_.getMagnet());
    }

    const auto mag_vec3 =  mag_bias_sum / config_.calibrate_times;
    return Quat(Vector3(0,0,-1), mag_vec3.normalized()).inverse();
}


void Estimator::calibrate(){
    bias_.acc = calculateAccelBias();
    bias_.gyro = calculateGyroBias();
    bias_.mag = calculateMagBias();
}

void Estimator::init(){
    if(config_.force_calibrate){
        calibrate();
    }else{
        bias_ = config_.pre_bias;
    }
}