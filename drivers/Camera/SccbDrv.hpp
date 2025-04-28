#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::hal{

class SccbDrv final {
private:
    hal::I2c & i2c_;
    hal::I2cSlaveAddr<7> slave_addr_;
public:
    SccbDrv(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_(i2c), slave_addr_(addr){};
    SccbDrv(const SccbDrv & other) = default;
    SccbDrv(SccbDrv && other) = default;

    hal::HalResult write_reg(const uint8_t addr, const uint16_t data);
    hal::HalResult read_reg(const uint8_t addr, uint16_t & data);
};

}