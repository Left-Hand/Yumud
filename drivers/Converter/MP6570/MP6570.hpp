#pragma once

#include <optional>

#include "mp6570_prelude.hpp"

namespace ymd::drivers::mp6570{
struct [[nodiscard]] MP6570_I2cTransport final{

    explicit MP6570_I2cTransport(const hal::I2cDrv & i2c_drv) : 
        i2c_drv_(i2c_drv) {}
    explicit MP6570_I2cTransport(hal::I2cDrv && i2c_drv) : 
        i2c_drv_(std::move(i2c_drv)) {}

    hal::HalResult write_reg(const uint8_t reg_addr, const uint16_t reg_val){
        return i2c_drv_.write_reg(reg_addr, reg_val, std::endian::big);
    }

    hal::HalResult read_reg(const uint8_t reg_addr, uint16_t & reg_val){
        return i2c_drv_.read_reg(reg_addr, reg_val, std::endian::big);
    }

private:
    hal::I2cDrv i2c_drv_;
};

struct [[nodiscard]] MP6570_SpiTransport final{
    explicit MP6570_SpiTransport(const SlaveAddress spi_slave_addr, const hal::SpiDrv & spi_drv)
        : spi_slave_addr_(spi_slave_addr), spi_drv_(spi_drv) {}
    explicit MP6570_SpiTransport(const SlaveAddress spi_slave_addr, hal::SpiDrv && spi_drv)
        : spi_slave_addr_(spi_slave_addr), spi_drv_(std::move(spi_drv)) {}


    hal::HalResult write_reg(const uint8_t reg_addr, const uint16_t reg_val);

    hal::HalResult read_reg(const uint8_t reg_addr, uint16_t & reg_val);
private:
    SlaveAddress spi_slave_addr_ ;
    hal::SpiDrv spi_drv_;
};


}