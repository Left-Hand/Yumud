#pragma once

#include "core/utils/sumtype.hpp"

namespace ymd::hal::can{



/// @brief CAN发送事件
enum class [[nodiscard]] TransmitEventType:uint8_t{
    Failed,
    Success
};

/// @brief CAN接收事件
enum class [[nodiscard]] ReceiveEventType:uint8_t{
    Pending,
    Full,
    Overrun,
};

/// @brief CAN状态事件
enum class [[nodiscard]] StatusEventType:uint8_t{
    Wakeup,
    SleepAcknowledge,
    ErrorWarning,
    ErrorPassive,
    BusOff,
    LastErrorCode,
    Error
};

struct [[nodiscard]] Event:public Sumtype<TransmitEventType, ReceiveEventType, StatusEventType>{};


}

namespace ymd::hal{

using CanEvent = can::Event;
using CanTransmitEventType = can::TransmitEventType;
using CanReceiveEventType = can::ReceiveEventType;
using CanStatusEventType = can::StatusEventType;
}