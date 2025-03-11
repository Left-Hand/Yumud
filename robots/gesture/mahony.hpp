#pragma once

#include "sys/math/real.hpp"
#include "types/quat/quat.hpp"


namespace ymd{
class Mahony{
public:
    using Quat = Quat_t<real_t>;
    using Vector3 = Vector3_t<real_t>;
protected:
    real_t invSampleFreq = real_t(0.005);
    real_t twoKp = 2;											// 2 * proportional gain (Kp)
    real_t twoKi = 2;											// 2 * integral gain (Ki)
    real_t integralFBx = 0;
    real_t integralFBy = 0; 
    real_t integralFBz = 0;	// 积分误差

	Quat q;
public:

    [[nodiscard]] Quat update(const Vector3 & gyr,const Vector3 & acc);
    [[nodiscard]] Quat update9(const Vector3 & gyr,const Vector3 & acc,const Vector3 & mag);
};

}