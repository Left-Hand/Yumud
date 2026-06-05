#pragma once

//这个驱动已经完成了基础使用

#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"

#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/InvensenseIMU.hpp"

namespace ymd::drivers{

struct MPU8658_Prelude{
    using Error = ImuError;
    
    template<typename T = void>
    using IResult = Result<T, Error>;

};

}