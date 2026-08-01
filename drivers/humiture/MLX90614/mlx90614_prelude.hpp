#pragma once

// MLX90614 系列模块是一组通用的红外测温模块。在出厂前该模块已进行校验及线性化，
// 具有非接触、体积小、精度高，成本低等优点。被测目标温度和环境温度能通过单通道输出，
// 并有两种输出接口，适合于汽车空调、室内暖气、家用电器、手持设备以及医疗设备应用等。
// 测温方式可分为接触式和非接触式，接触式测温只能测量被测物体与测温传感器达到热平衡后的温度，
// 所以响应时间长，且极易受环境温度的影响；而红外测温是根据被测物体的红外辐射能量来确定物体的温度，
// 不与被测物体接触，具有影响动被测物体温度分布场，
// 温度分辨率高、响应速度快、测温范围广、不受测温上限的限制、稳定性好等特点

// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/mlx90614-non-contact-temp-sensor.html

#include "core/utils/reg_base.hpp"
#include "core/math/real.hpp"
#include "middlewares/algebra/vectors/vec3.hpp"

#include "core/utils/result.hpp"
#include "core/utils/errno.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"
#include "hal/conn/spi/spidrv.hpp"

namespace ymd::drivers{


struct MLX90614_Prelude{
    enum class Error_Kind{
    };


    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    
    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x0C >> 1);

};


};