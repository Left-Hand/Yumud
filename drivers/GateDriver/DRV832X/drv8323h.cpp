#include "drv8323h.hpp"


#define DRV832X_DEBUG_EN

#ifdef DRV832X_DEBUG_EN
#define DRV832X_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV832X_PANIC(...) PANIC(__VA_ARGS__)
#define DRV832X_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define DRV832X_DEBUG(...)
#define DRV832X_PANIC(...)  PANIC()
#define DRV832X_ASSERT(cond, ...) ASSERT(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

using Error = DRV8323H::Error;

template<typename T = void>
using IResult = Result<T, Error>;



IResult<> DRV8323H::init(const Config & cfg){

    TODO();
    return Ok();
}

IResult<> DRV8323H::reconf(const Config & cfg){
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_peak_current(const PeakCurrent peak_current){
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_ocp_mode(const OcpMode ocp_mode){
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_gain(const Gain gain){
    TODO();
    return Ok();
}

IResult<> DRV8323H::enable_pwm3(const Enable en){
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_drive_hs(const IDriveP pdrive, const IDriveN ndrive){
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_drive_ls(const IDriveP pdrive, const IDriveN ndrive){
    TODO();
    return Ok();
}

IResult<> DRV8323H::set_drive_time(const PeakDriveTime ptime){
    TODO();
    return Ok();
}
