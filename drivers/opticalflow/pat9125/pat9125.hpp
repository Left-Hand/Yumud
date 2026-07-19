#pragma once


#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

//参考资料：
// https://github.com/r21m/PAT9125-arduino-single/blob/master/pat9125.h

namespace ymd::drivers{

struct PAT9125_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x52 >> 1);


    enum class [[nodiscard]] Error_Kind:uint8_t{
        InvalidChipId
    };
    
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
    


    enum class [[nodiscard]] RegAddr : uint8_t{
        Pid1            = 0x00,
        Pid2            = 0x01,
        Motion          = 0x02,
        DeltaXl        = 0x03,
        DeltaYl        = 0x04,
        Mode            = 0x05,
        Config          = 0x06,
        Wp              = 0x09,
        Sleep1          = 0x0a,
        Sleep2          = 0x0b,
        ResX           = 0x0d,
        ResY           = 0x0e,
        DeltaXyh       = 0x12,
        Shutter         = 0x14,
        Frame           = 0x17,
        Orientation     = 0x19,
        BankSelection  = 0x7f,
    };
};



}