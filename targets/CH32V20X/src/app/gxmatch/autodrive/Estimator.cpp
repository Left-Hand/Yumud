#include "Estimator.hpp"

using namespace ymd::drivers;
using namespace gxm;

void Estimator::update(){
    acc_sensor_.update();
    gyro_sensor_.update();
    mag_sensor_.update();

    acc3_ = Vector3(acc_sensor_.getAccel()) + config_.acc_trim;
    gyro3_ = Vector3(gyro_sensor_.getGyro()) + config_.gyro_trim;
}

Vector3 Estimator::calculateAccelTrim(){
    Vector3 acc_trim = Vector3::ZERO;
    
    for(size_t i = 0; i < config_.trim_times; i++){
        acc_sensor_.update();
        acc_trim += Vector3(acc_sensor_.getAccel());
    }

    return acc_trim / config_.trim_times;
}



Vector3 Estimator::calculateGyroTrim(){
    Vector3 gyro_trim = Vector3::ZERO;
    
    for(size_t i = 0; i < config_.trim_times; i++){
        gyro_sensor_.update();
        gyro_trim += Vector3(gyro_sensor_.getGyro());
    }

    return gyro_trim / config_.trim_times;
}
