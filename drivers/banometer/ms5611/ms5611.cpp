#include "ms5611.hpp"



// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/ms5611-pressure-sensor.html


using namespace ymd::drivers;


using Self = MS5611;

using Error = Self::Error;

template<typename T = void>
using IResult = Self::IResult<T>;


IResult<> MS5611_Transport::read_u24(uint32_t & val){
    return Ok();
}