#pragma once


#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/typetraits/enum_traits.hpp"

#include "types/vectors/vector3/Vector3.hpp"
#include "hal/hal_result.hpp"

namespace ymd::drivers{

namespace details{
enum class ImuError_Kind:uint8_t{
    WrongWhoAmI,
    PackageNotMatch,
    UnknownDevice,
    WrongDeviceId,
    WrongCompanyId,
    PhyVerifyFailed,

    CantSetup,
    
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
    AxisZOverflow
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
    virtual Result<Vector3<q24>, ImuError> read_acc() = 0;
};

class GyroscopeIntf{
public:
    virtual Result<Vector3<q24>, ImuError> read_gyr() = 0;
};

class MagnetometerIntf{
public:

    virtual Result<Vector3<q24>, ImuError> read_mag() = 0;
};

}

namespace ymd{
    DERIVE_DEBUG(drivers::details::ImuError_Kind)
// OutputStream & print_halerr_kind(OutputStream & os, const drivers::details::ImuError_Kind err){
//     derive_debug_dispatcher<drivers::details::ImuError_Kind>::call(os, err);
//     return os;
// }

}