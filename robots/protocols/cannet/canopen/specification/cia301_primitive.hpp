#pragma once

#include "../canopen_primitive/canopen_primitive.hpp"

namespace ymd::canopen{
using namespace canopen::primitive;



enum class NmtCommand:uint8_t{
    StartRemoteNode = 0x01,
    StopRemoteNode = 0x02,
    EnterPreOperational = 0x80,
    ResetNode = 0x81,
    ResetCommunication = 0x82
};
}