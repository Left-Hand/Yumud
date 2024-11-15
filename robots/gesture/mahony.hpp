#pragma once

#include "sys/math/real.hpp"
#include "types/quat/quat_t.hpp"


namespace ymd{
class Mahony{
protected:
    real_t sampleFreq = 1000;
    real_t twoKp = 2;											// 2 * proportional gain (Kp)
    real_t twoKi = 2;											// 2 * integral gain (Ki)
    real_t integralFBx = 0;
    real_t integralFBy = 0; 
    real_t integralFBz = 0;	// 积分误差
public:
    using Quat = Quat_t<real_t>;
    using Vector3 = Vector3_t<real_t>;
    Quat update(const Vector3 & gyro,const Vector3 & accel);
};

}