#pragma once

#include <cstdint>
#include <optional>

#include "core/io/regs.hpp"
#include "core/clock/clock.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "primitive/colors/rgb/rgb.hpp"


namespace ymd::drivers{
namespace details{
enum class DisplayerError_Kind:uint8_t{
    CantSetup,
    NoAvailablePhy
};
DEF_DERIVE_DEBUG(DisplayerError_Kind)
}
DEF_ERROR_SUMWITH_HALERROR(DisplayerError, details::DisplayerError_Kind)

template<typename T>
struct DrawTarget;
}

namespace ymd::drivers::displayer::prelude{

enum class Orientation:uint8_t{
    Portrait,
    Landscape,
    PortraitSwapped,
    LandscapeSwapped,
};


}