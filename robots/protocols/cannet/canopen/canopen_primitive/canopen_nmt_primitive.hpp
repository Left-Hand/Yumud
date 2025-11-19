#pragma once


#include "canopen_primitive_base.hpp"
#include "core/tmp/bits/width.hpp"

namespace ymd::canopen::primitive{

enum class NodeState:uint8_t{
    BootUp = 0x00,
    Stopped = 0x04,
    PreOperational = 0x05,
    Operating = 0x07,
};


enum class NmtCommand:uint8_t{
    StartRemoteNode = 0x01,
    StopRemoteNode = 0x02,
    EnterPreOperational = 0x80,
    ResetNode = 0x81,
    ResetCommunication = 0x82
};



}