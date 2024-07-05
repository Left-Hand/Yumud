#pragma once

#include "sys/platform.h"
#include "hal/gpio/port.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"

#include <memory>
#include <functional>

namespace CanUtils{
    enum class BaudRate{
        Kbps125,
        Kbps250,
        Kbps500,
        Mbps1
    };

    enum class Mode{
        Normal = CAN_Mode_Normal,
        Silent = CAN_Mode_Silent,
        Internal = CAN_Mode_Silent_LoopBack,
        Loopback = CAN_Mode_LoopBack
    };

};
