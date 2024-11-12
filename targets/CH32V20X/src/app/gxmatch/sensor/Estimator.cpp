#include "Estimator.hpp"

using namespace yumud::drivers;
using namespace gxm;

void gxm::Estimator::update(){
    acc_sensor_.update();
    gyro_sensor_.update();
    mag_sensor_.update();

    acc3_ = Vector3(acc_sensor_.getAccel()) + config_.acc_trim;
    gyro3_ = Vector3(gyro_sensor_.getGyro()) + config_.gyro_trim;
}
