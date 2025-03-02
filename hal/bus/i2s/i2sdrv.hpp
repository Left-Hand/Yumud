#pragma once

#include "i2s.hpp"
#include "../busdrv.hpp"
#include <type_traits>
#include <initializer_list>

namespace ymd::hal{

class I2sDrv:public BusDrv<I2s>{
public:
    I2sDrv(I2s & _bus, const uint8_t _index = 0):BusDrv<I2s>(_bus, _index){;}
};

}