#pragma once

#include "core/io/regs.hpp"
#include "types/vector3/Vector3.hpp"

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"

#include "core/math/real.hpp"



namespace ymd::drivers{

class AccelerometerIntf{
public:
    virtual Option<Vector3_t<real_t>> get_acc() = 0;
};

class GyroscopeIntf{
public:
    virtual Option<Vector3_t<real_t>> get_gyr() = 0;
};

class MagnetometerIntf{
public:

    virtual Option<Vector3_t<real_t>> get_magnet() = 0;
};

class Axis6:public AccelerometerIntf, public GyroscopeIntf{
public:
};

class Axis9:public Axis6, public MagnetometerIntf{

};

}