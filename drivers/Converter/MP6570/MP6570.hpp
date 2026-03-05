#pragma once

#include <optional>

#include "mp6570_prelude.hpp"

namespace ymd::drivers::mp6570{
struct [[nodiscard]] MP6570_I2cTransport final{

    explicit MP6570_I2cTransport(const hal::I2cDrv & i2c_drv) : 
        i2c_drv_(i2c_drv) {}
    explicit MP6570_I2cTransport(hal::I2cDrv && i2c_drv) : 
        i2c_drv_(std::move(i2c_drv)) {}

    hal::HalResult write_reg(const uint8_t reg_addr, const uint16_t data){
        return i2c_drv_.write_reg(reg_addr, data, std::endian::big);
    }

    hal::HalResult read_reg(const uint8_t reg_addr, uint16_t & data){
        return i2c_drv_.read_reg(reg_addr, data, std::endian::big);
    }

private:
    hal::I2cDrv i2c_drv_;
};

struct [[nodiscard]] MP6570_SpiTransport final{
    explicit MP6570_SpiTransport(const SlaveAddress spi_slave_addr, const hal::SpiDrv & spi_drv)
        : spi_slave_addr_(spi_slave_addr), spi_drv_(spi_drv) {}
    explicit MP6570_SpiTransport(const SlaveAddress spi_slave_addr, hal::SpiDrv && spi_drv)
        : spi_slave_addr_(spi_slave_addr), spi_drv_(std::move(spi_drv)) {}


    hal::HalResult write_reg(const uint8_t reg_addr, const uint16_t data){
        const TxPacket packet = TxPacket::from_write({
            .slave_addr = spi_slave_addr_,
            .reg_addr = reg_addr,
            .pen = true,
            .data = data,
        });

        return spi_drv_.write_burst<uint8_t>(
            std::span(packet.payload_bytes),
            DISC);
    }

    hal::HalResult read_reg(const uint8_t reg_addr, uint16_t & data){
        const TxPacket packet = TxPacket::from_read({
            .slave_addr = spi_slave_addr_,
            .reg_addr = reg_addr,
            .pen = true,
        });

        if(const auto res = spi_drv_.write_burst<uint8_t>(
            std::span(packet.payload_bytes),
            CONT); res.is_err()) return res;

        std::array<uint8_t, 3> rx_bytes;

        if(const auto res = spi_drv_.read_burst<uint8_t>(
            std::span(rx_bytes),
            DISC); res.is_err()) return res;

        if(const auto may_data = rx_bytes_to_data(std::span(rx_bytes)); 
            may_data.is_none()
        ){
            PANIC{};
        } else{
            data = may_data.unwrap();
        }
    }

private:
    SlaveAddress spi_slave_addr_ = 0;
    hal::SpiDrv spi_drv_;
};


}