#pragma once
#include "drivers/device_defs.h"
#include "types/vector3/Vector3.hpp"

#include "sys/utils/Option.hpp"
#include "sys/utils/Result.hpp"

#define REG16I_QUICK_DEF(addr, typename, name)\
struct typename :public Reg16i<>{scexpr uint8_t address = addr; int16_t :16;} name = {};

#define REG16_QUICK_DEF(addr, typename, name)\
struct typename :public Reg16<>{scexpr RegAddress address = addr; int16_t :16;} name = {};

#define REG8_QUICK_DEF(addr, typename, name)\
struct typename :public Reg16i<>{scexpr RegAddress address = addr; int16_t :16;} name = {};



namespace ymd::drivers{

class Accelerometer{
public:
    virtual std::tuple<real_t, real_t, real_t> getAcc() = 0;
};

class Gyroscope{
public:
    virtual std::tuple<real_t, real_t, real_t>  getGyr() = 0;
};

class Magnetometer{
public:

    virtual std::tuple<real_t, real_t, real_t> getMagnet() = 0;
};

class Axis6:public Accelerometer, public Gyroscope{
public:
};

class Axis9:public Axis6, public Magnetometer{

};

}