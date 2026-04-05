#include "drv8323h.hpp"


#define DRV832X_DEBUG_EN

#ifdef DRV832X_DEBUG_EN
#define DRV832X_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV832X_PANIC(...) PANIC(__VA_ARGS__)
#define DRV832X_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define DRV832X_DEBUG(...)
#define DRV832X_PANIC(...)  PANIC_NSRC()
#define DRV832X_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

using Error = DRV8323H::Error;

template<typename T = void>
using IResult = Result<T, Error>;



IResult<> DRV8323H::init(const Config & cfg){
    (void)(cfg);
    TODO();
    return Ok();
}

IResult<> DRV8323H::reconf(const Config & cfg){
    (void)(cfg);
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_peak_current(const PeakCurrent peak_current){
    (void)(peak_current);
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_ocp_mode(const OcpMode ocp_mode){
    (void)(ocp_mode);
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_gain(const Gain gain){
    (void)(gain);
    TODO();
    return Ok();
}

IResult<> DRV8323H::enable_pwm3(const Enable en){
    (void)(en);
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_drive_hs(const IDriveP pdrive, const IDriveN ndrive){
    (void)(pdrive);
    (void)(ndrive);
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_drive_ls(const IDriveP pdrive, const IDriveN ndrive){
    (void)(pdrive);
    (void)(ndrive);
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_drive_time(const PeakDriveTime ptime){
    (void)(ptime);
    TODO();
    return Ok();
}
