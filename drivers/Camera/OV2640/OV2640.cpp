#include "OV2640.hpp"


using namespace ymd;
using namespace ymd::drivers;

using Self = OV2640;
using Error = OV2640::Error;

template<typename T = void> 
using IResult = Result<T, Error>;

IResult<> OV2640::init(){
    return Ok();
}

IResult<> OV2640::validate(){
    return Ok();
}