#ifndef __IMU_HPP__

#define __IMU_HPP__

#include "sys/platform.h"
#include "drivers/device_defs.h"
#include "types/real.hpp"

class Accelerometer{
public:
    virtual void getAccel(real_t & x, real_t & y, real_t & z) = 0;
};

class Gyroscope{
public:
    virtual void getGyro(real_t & x, real_t & y, real_t & z) = 0;
};

class Magnetometer{
public:
    virtual void getMagnet(real_t & x, real_t & y, real_t & z) = 0;
};

class Axis6:public Accelerometer, public Gyroscope{
public:
    virtual void flush() = 0;
};

class Axis9:public Axis6, public Magnetometer{

};

#endif