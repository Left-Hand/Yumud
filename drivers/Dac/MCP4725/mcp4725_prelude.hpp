#pragma once

// https://blog.csdn.net/qq_43547520/article/details/131880984


#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/Result.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct MCP4725_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u8(0b1100'001);

    static constexpr size_t VREF_5V = 5000;

    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

}