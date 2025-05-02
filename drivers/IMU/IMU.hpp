#pragma once


#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "types/vector3/Vector3.hpp"

#include "hal/hal_result.hpp"

namespace ymd::drivers{

namespace details{
enum class ImuError_Kind:uint8_t{
    WrongWhoAmI,
    PackageNotMatch,
    UnknownDevice,
    PhyVerifyFailed,

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
};
}

DEF_ERROR_SUMWITH_HALERROR(ImuError, details::ImuError_Kind)
}
namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::ImuError, hal::HalResult> {
        static Result<T, drivers::ImuError> convert(const hal::HalResult res){
            
            if(res.is_ok()) return Ok();
            return Err(res.unwrap_err()); 
        }
    };
}

namespace ymd::drivers{
class AccelerometerIntf{
public:
    virtual Option<Vector3_t<real_t>> get_acc() = 0;
};

class GyroscopeIntf{
public:
    virtual Option<Vector3_t<real_t>> get_gyr() = 0;
};

class MagnetometerIntf{
public:

    virtual Option<Vector3_t<real_t>> get_magnet() = 0;
};

}