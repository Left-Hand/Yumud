#pragma once


#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/hal_result.hpp"


namespace ymd::drivers{

namespace details{
enum class EncoderError_Kind:uint8_t{
    CantSetup,
    WrongPc,
    WrongCrc,
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

    SpiIsNotSupportedYet,
    I2cIsNotSupportedYet,
    NoSupportedPhy,
    Unreachable
};
DERIVE_DEBUG(EncoderError_Kind)
}

DEF_ERROR_SUMWITH_HALERROR(EncoderError, details::EncoderError_Kind)


class EncoderIntf{
public:
    virtual Result<real_t, EncoderError> read_lap_position() = 0;
    virtual Result<void, EncoderError> update() = 0;

    // FRIEND_DERIVE_DEBUG(details::EncoderError_Kind);
    virtual ~EncoderIntf() = default;
};


class IncrementalEncoderIntf: public EncoderIntf{

};

class AbsoluteEncoderIntf: public EncoderIntf{
protected:
    // virtual void forward() = 0;
    // virtual void backward() = 0;
};

}