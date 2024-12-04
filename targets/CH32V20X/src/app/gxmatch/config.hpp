#include "misc.hpp"

namespace gxm{

struct SysConfig{
    JointConfig joint_config;
    Scara::Config scara_config;
    ZAxisCross::Config zaxis_config;
    GrabModule::Config grab_config;
    // WheelStepper::Config wheel_config;
    Wheel::Config wheel_config;
    Estimator::Config est_config;
};


SysConfig create_default_config();

}