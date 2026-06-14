#pragma once

// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/mlx90614-non-contact-temp-sensor.html

#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "algebra/vectors/vec3.hpp"

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