#pragma once

#include "core/utils/sumtype.hpp"

namespace ymd::hal::can{



/// @brief CAN发送事件
enum class [[nodiscard]] TransmitEvent:uint8_t{
    Failed,
    Success
};

/// @brief CAN接收事件
enum class [[nodiscard]] ReceiveEvent:uint8_t{
    Fifo0Pending,
    Fifo0Full,
    Fifo0Overrun,
    Fifo1Pending,
    Fifo1Full,
    Fifo1Overrun,
};

/// @brief CAN状态事件
enum class [[nodiscard]] StatusEvent:uint8_t{
    Wakeup,
    SleepAcknowledge,
    ErrorWarning,
    ErrorPassive,
    BusOff,
    LastErrorCode,
    Error
};

struct [[nodiscard]] Event:public Sumtype<TransmitEvent, ReceiveEvent, StatusEvent>{};


}

namespace ymd::hal{

using CanEvent = can::Event;
using CanTransmitEvent = can::TransmitEvent;
using CanReceiveEvent = can::ReceiveEvent;
using CanStatusEvent = can::StatusEvent;
}