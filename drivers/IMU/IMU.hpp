#pragma once
#include "drivers/device_defs.h"

namespace ymd::drivers{

class IMU_Base{
protected:
    struct Vec3i16{
        int16_t x;
        int16_t y;
        int16_t z;
    };
public:
    virtual void update() = 0;
};

class Accelerometer:virtual public IMU_Base{
public:
    virtual std::tuple<real_t, real_t, real_t> getAccel() = 0;
};

class Gyroscope:virtual public IMU_Base{
public:
    virtual std::tuple<real_t, real_t, real_t>  getGyro() = 0;
};

class Magnetometer:virtual public IMU_Base{
public:

    virtual std::tuple<real_t, real_t, real_t> getMagnet() = 0;
};

class Axis6:public Accelerometer, public Gyroscope{
public:
};

class Axis9:public Axis6, public Magnetometer{

};

}