#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>

#include "sccb.hpp"
#include "core/utils/Option.hpp"


namespace ymd::hal{

class SccbDrv final {
public:
    explicit SccbDrv(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_(i2c.deref()), slave_addr_(addr){};
    SccbDrv(const SccbDrv & other) = default;
    SccbDrv(SccbDrv && other) = default;

    [[nodiscard]] hal::HalResult write_reg(const uint8_t addr, const uint16_t data);
    [[nodiscard]] hal::HalResult read_reg(const uint8_t addr, uint16_t & data);
private:
    hal::I2cBase & i2c_;
    hal::I2cSlaveAddr<7> slave_addr_;
};

}