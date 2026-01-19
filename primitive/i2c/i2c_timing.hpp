#pragma once

#include "i2c_event.hpp"
#include "primitive/nearest_freq.hpp"

namespace ymd::hal::i2c{

//TODO
struct Prescaler{

    
};


using Baudrate = Sumtype<Prescaler, NearestFreq, LeastFreq>;
}

namespace ymd::hal{
using I2cBuadrate = hal::i2c::Baudrate;
}