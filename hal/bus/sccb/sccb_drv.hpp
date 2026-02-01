#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>

#include "sccb.hpp"
#include "core/utils/Option.hpp"


namespace ymd::hal{

class SccbDrv final {
public:
    explicit SccbDrv(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> i2c_addr):
        i2c_(i2c.deref()), slave_addr_(i2c_addr){};
    SccbDrv(const SccbDrv & other) = default;
    SccbDrv(SccbDrv && other) = default;

    [[nodiscard]] hal::HalResult write_reg(const uint8_t reg_addr, const uint16_t reg_val);
    [[nodiscard]] hal::HalResult read_reg(const uint8_t reg_addr, uint16_t & reg_val);
private:
    hal::I2cBase & i2c_;
    hal::I2cSlaveAddr<7> slave_addr_;
};

}