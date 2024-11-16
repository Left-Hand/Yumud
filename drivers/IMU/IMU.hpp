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

class Accerometer:virtual public IMU_Base{
public:
    virtual std::tuple<real_t, real_t, real_t> getAcc() = 0;
};

class Gyrscope:virtual public IMU_Base{
public:
    virtual std::tuple<real_t, real_t, real_t>  getGyr() = 0;
};

class Magnetometer:virtual public IMU_Base{
public:

    virtual std::tuple<real_t, real_t, real_t> getMagnet() = 0;
};

class Axis6:public Accerometer, public Gyrscope{
public:
};

class Axis9:public Axis6, public Magnetometer{

};

}