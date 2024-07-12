#ifndef __IMU_HPP__

#define __IMU_HPP__

#include "../sys/core/platform.h"
#include "../types/real.hpp"

class Accelerometer{
public:
    virtual std::tuple<real_t, real_t, real_t> getAccel() = 0;
};

class Gyroscope{
public:
    virtual std::tuple<real_t, real_t, real_t>  getGyro() = 0;
};

class Magnetometer{
public:
    virtual std::tuple<real_t, real_t, real_t> getMagnet() = 0;
};

class Axis6:public Accelerometer, public Gyroscope{
public:
    virtual void update() = 0;
};

class Axis9:public Axis6, public Magnetometer{

};

#endif