#ifndef __I2C_DRV_HPP__
#define __I2C_DRV_HPP__

#include "i2c.hpp"
#include "../busdrv.hpp"
#include <type_traits>
#include <initializer_list>

class I2cDrv:public ProtocolBusDrv<I2c>{
public:
    I2cDrv(I2c & _bus, const uint8_t _index = 0):ProtocolBusDrv<I2c>(_bus, _index){;}



};

#endif