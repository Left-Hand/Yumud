#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "core/clock/clock.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "types/rgb.h"


namespace ymd::drivers{
namespace details{
enum class DisplayerError_Kind:uint8_t{
    CantSetup,
    WrongPc,
    MagnetLost,
    MagnetWeak,
    MagnetOverflow
};
}
DEF_ERROR_SUMWITH_HALERROR(DisplayerError, details::DisplayerError_Kind)
}


namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::DisplayerError, hal::HalResult> {
        static Result<T, drivers::DisplayerError> convert(const hal::HalResult res){
            
            if(res.is_ok()) return Ok();
            return Err(res.unwrap_err()); 
        }
    };
}
