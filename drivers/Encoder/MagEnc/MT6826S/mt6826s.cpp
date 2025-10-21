#include "mt6826s_prelude.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = MT6826S::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<> MT6826S::init(){
    TODO();
    return Ok();
}

IResult<Angle<q31>> MT6826S::get_lap_angle(){
    TODO();
    return Ok(Angle<q31>::ZERO);
}

IResult<MagStatus> MT6826S::get_mag_status(){
    TODO();
    return Ok(MagStatus::from_proper());
}

