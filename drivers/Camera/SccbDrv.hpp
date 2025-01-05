#pragma once

#include "hal/bus/i2c/i2c.hpp"
#include "sys/debug/debug_inc.h"
#include "hal/bus/busdrv.hpp"

#include <type_traits>
#include <concepts>
#include <initializer_list>


namespace ymd::hal{

class SccbDrv:public BusDrv<I2c> {
protected:
    using BusDrv<I2c>::index;
    using BusDrv<I2c>::bus;
public:
    SccbDrv(I2c & i2c, const uint8_t addr):BusDrv(i2c, addr){};
    SccbDrv(const SccbDrv & other) = default;
    SccbDrv(SccbDrv && other) = default;


    void writeReg(const uint8_t reg_address, const uint16_t reg_data);
    void readReg(const uint8_t reg_address, uint16_t & reg_data);
};

}