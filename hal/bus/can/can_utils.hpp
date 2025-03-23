#pragma once

#include "core/platform.hpp"
#include "core/sdk.hpp"

#include <memory>
#include <functional>

namespace ymd::hal{
    enum class CanBaudrate:uint8_t{
        _125K,
        _250K,
        _500K,
        _1M
    };

    enum class CanMode:uint8_t{
        Normal = CAN_Mode_Normal,
        Silent = CAN_Mode_Silent,
        Internal = CAN_Mode_Silent_LoopBack,
        Loopback = CAN_Mode_LoopBack
    };

    enum class CanError:uint8_t{
        Overrun,
        Bit,
        Stuff,
        Crc,
        Form,
        Acknowledge,
        Other,
    };

    enum class CanRemoteSpec:uint8_t{
        Data = 0,
        Any = 0,
        Remote = 1,
        Specified = 1
    };  

};

