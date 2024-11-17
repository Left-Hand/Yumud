#pragma once

#include "gxmatch.hpp"

#include "autodrive/Planner.hpp"
#include "autodrive/sequence/TrapezoidSolver_t.hpp"


#include "machine/scara/scara.hpp"
#include "machine/actuator/zaxis_stp.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/gpio/port_virtual.hpp"
#include "robots/foc/remote/remote.hpp"

#include "types/image/painter.hpp"
#include "hal/bus/i2c/i2csw.hpp"


#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/IMU/Axis6/BMI160/BMI160.hpp"
#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"
#include "drivers/IMU/Gyroscope/QMC5883L/qmc5883l.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "common/inc.hpp"
#include "machine/chassis_module.hpp"
#include "machine/grab_module.hpp"
#include "async/Node.hpp"

namespace gxm{

struct RobotConfig{

};

class Robot{
protected:
    const RobotConfig & config_;

    ChassisModule & chassis_module_;
    GrabModule & grab_module_;

    struct Refs{
        ChassisModule & chassis_module;
        GrabModule & grab_module;
    };

public:
    Robot(const Robot & other) = delete;
    Robot(Robot && other) = delete;

    Robot(const RobotConfig & config, const Refs & refs):
        config_(config), 
        chassis_module_(refs.chassis_module), grab_module_(refs.grab_module){;}

    void init();
};

}