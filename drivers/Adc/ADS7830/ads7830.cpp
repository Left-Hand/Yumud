#include "ads7830.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = ADS7830::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> init(){
    return Ok();
}

IResult<> validate(){
    return Ok();
}