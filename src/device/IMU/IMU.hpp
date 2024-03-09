#ifndef __IMU_HPP__

#define __IMU_HPP__

#include "src/platform.h"
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

};

class Axis9:public Accelerometer, public Magnetometer, public Gyroscope{

};

#endif