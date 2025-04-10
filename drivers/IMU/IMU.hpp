#pragma once

#include "core/io/regs.hpp"
#include "types/vector3/Vector3.hpp"

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"

#include "core/math/real.hpp"



namespace ymd::drivers{

class Accelerometer{
public:
    virtual Option<Vector3_t<real_t>> get_acc() = 0;
};

class Gyroscope{
public:
    virtual Option<Vector3_t<real_t>> get_gyr() = 0;
};

class Magnetometer{
public:

    virtual Option<Vector3_t<real_t>> get_magnet() = 0;
};

class Axis6:public Accelerometer, public Gyroscope{
public:
};

class Axis9:public Axis6, public Magnetometer{

};

}