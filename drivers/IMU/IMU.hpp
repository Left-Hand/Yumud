#pragma once


#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/magic/enum_traits.hpp"

#include "types/vectors/vector3.hpp"
#include "hal/hal_result.hpp"

namespace ymd::drivers{

namespace details{
enum class ImuError_Kind:uint8_t{
    PackageNotMatch,
    UnknownDevice,
    WrongWhoAmI,
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

    NoAvailablePhy,
    I2cPhyIsNotImplementedYet,
    I3cPhyIsNotImplementedYet,
    SpiPhyIsNotImplementedYet,

    Unreachable
};
DEF_DERIVE_DEBUG(ImuError_Kind)
}
DEF_ERROR_SUMWITH_HALERROR(ImuError, details::ImuError_Kind)
}


namespace ymd::drivers{
class AccelerometerIntf{
public:
    template<typename T>
    static constexpr T GRAVITY_ACC = T(9.80665);

    virtual Result<Vec3<q24>, ImuError> read_acc() = 0;
};

class GyroscopeIntf{
public:
    virtual Result<Vec3<q24>, ImuError> read_gyr() = 0;
};

class MagnetometerIntf{
public:

    virtual Result<Vec3<q24>, ImuError> read_mag() = 0;
};

}
