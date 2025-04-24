#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::hal{

class SccbDrv:public BusDrv<I2c> {
protected:
    using BusDrv<I2c>::index;
    using BusDrv<I2c>::bus;
public:
    SccbDrv(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr):
        BusDrv(i2c, addr.as_u8()){};
    SccbDrv(const SccbDrv & other) = default;
    SccbDrv(SccbDrv && other) = default;


    void write_reg(const uint8_t addr, const uint16_t data);
    void read_reg(const uint8_t addr, uint16_t & data);
};

}