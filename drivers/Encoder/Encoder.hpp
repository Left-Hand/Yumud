#pragma once


#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "core/utils/Errno.hpp"


#include "primitive/hal_result.hpp"


namespace ymd::drivers{

namespace details{
enum class EncoderError_Kind:uint8_t{
    UnknownPackage,
    EEpromProgramFailed,
    CantSetup,

    InvalidPc,
    InvalidPc2,
    InvalidCrc,

    MagnetLost,
    MagnetInvalid,
    MagnetLow,
    MagnetHigh,

    OverSpeed,
    RegProgramFailed,
    RegProgramResponseFormatInvalid,
    ValueOverflow,
    ValueUnderflow,
    InvalidRxFormat,

    SpiIsNotImplementedYet,
    I2cIsNotImplementedYet,

    UnderVoltage,
    Unreachable
};
}
DEF_ERROR_SUMWITH_HALERROR(EncoderError, details::EncoderError_Kind)
}

namespace ymd{
OutputStream& operator << (OutputStream& os, const drivers::details::EncoderError_Kind value);
}


namespace ymd::drivers{
class EncoderIntf{
public:
    virtual Result<Angular<uq32>, EncoderError> read_lap_angle() = 0;
    virtual Result<void, EncoderError> update() = 0;
    virtual ~EncoderIntf() = default;
};


}

