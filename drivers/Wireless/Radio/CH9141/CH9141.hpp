#pragma once

#include "hal/gpio/gpio_intf.hpp"

#include "core/clock/clock.hpp"

#include "core/utils/Option.hpp"

#if 0

namespace ymd::drivers{


class CH9141 final{
public:
    enum class PowerMode{
        Low,
        Quiest,
        Normal,
        LongDist
    };

    enum class Power{
        dBmN1 = 1,
        dBm2, dBm5, dBm8, dBm11, dBm14, dBm17, dBm20
    };

    enum class ErrCode:uint8_t{
        OK,
        BUF,
        PARAM,
        DENIED,
        BUSY
    };
};
};

#endif