#pragma once

#include <tuple>

#include "core/utils/reg_base.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"


// 参考资料:
// https://item.taobao.com/item.htm?from=cart&id=938073201165

namespace ymd::drivers{

struct TOF10120_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x52 >> 1);


    enum class [[nodiscard]] Error_Kind:uint8_t{
        InvalidRangeMode,
        InvalidCommPort
    };
    
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
    

    enum class [[nodiscard]] RegAddr:uint8_t{
        RealtimeDistance = 0x00,
        FilteredDistance = 0x04,
        DistanceDiff = 0x06,
        RangeMode = 0x08,
        CommPort = 0x09
    };

    enum class RangeMode:uint8_t{
        Filtered,
        Unfiltered
    };

    enum class CommPort:uint8_t{
        Serial,
        I2c
    };
};


};