#pragma once

#include "sys/core/platform.h"
#include "hal/gpio/port.hpp"
#include "types/buffer/ringbuf/Fifo_t.hpp"

#include <memory>
#include <functional>

namespace ymd::hal::CanUtils{
    enum class BaudRate:uint8_t{
        _125K,
        _250K,
        _500K,
        _1M
    };

    enum class Mode:uint8_t{
        Normal = CAN_Mode_Normal,
        Silent = CAN_Mode_Silent,
        Internal = CAN_Mode_Silent_LoopBack,
        Loopback = CAN_Mode_LoopBack
    };

    enum class ErrCode:uint8_t{
        OK = CAN_ErrorCode_NoErr,
        STUFF_ERR = CAN_ErrorCode_StuffErr,
        FORM_ERR = CAN_ErrorCode_FormErr,
        ACK_ERR = CAN_ErrorCode_ACKErr,
        BIT_RECESSIVE_ERR = CAN_ErrorCode_BitRecessiveErr,
        BIT_DOMINANT_ERR = CAN_ErrorCode_BitDominantErr,
        CRC_ERR = CAN_ErrorCode_CRCErr,
        SOFTWARE_SET_ERR = CAN_ErrorCode_SoftwareSetErr
    };

    enum class RemoteType:uint8_t{
        Data = 0,
        Any = 0,
        Remote = 1,
        Specified = 1
    };  
};


#ifndef CAN_SOFTFIFO_SIZE
#define CAN_SOFTFIFO_SIZE 8
#endif