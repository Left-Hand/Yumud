#pragma once


#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"


#include "algebra/vectors/vec3.hpp"
#include "primitive/hal_result.hpp"

namespace ymd::drivers{
template<typename T>
static constexpr T GRAVITY_ACC = T(9.80665);

namespace details{
enum class ImuError_Kind:uint8_t{
    PackageNotMatch,
    UnknownDevice,
    InvalidChipId,
    WrongCompanyId,
    WrongPmuMode,

    AccCantSetup,
    GyrCantSetup,
    MagCantSetup,

    AccSelftestFailed,
    GyrSelftestFailed,
    DmpSelftestFailed,

    MagSelfdriveFailed,
    MagLost,
    MagWeak,

    MagOverflow,
    AccOverflow,
    GyrOverflow,

    AxisXOverflow,
    AxisYOverflow,
    AxisZOverflow,

    InvalidTemperature,

    NoAvailablePhy,
    I2cPhyIsNotImplementedYet,
    I3cPhyIsNotImplementedYet,
    SpiPhyIsNotImplementedYet,

    Unreachable
};
}
DEF_ERROR_SUMWITH_HALERROR(ImuError, details::ImuError_Kind)
}

namespace ymd{
OutputStream& operator << (OutputStream& os, const drivers::details::ImuError_Kind value);
}
