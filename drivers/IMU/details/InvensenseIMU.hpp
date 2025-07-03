#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

#include "core/utils/result.hpp"

namespace ymd::drivers{

class InvensenseSensor_Phy final{
public:
    using Error = ImuError;
    [[nodiscard]] Result<void, Error> write_reg(const uint8_t addr, const uint8_t data) {
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->write_reg(uint8_t(addr), data));
        }else if(spi_drv_){
            return Result<void, Error>(spi_drv_->write_single<uint8_t>(uint8_t(addr & 0x7f), CONT)
            | spi_drv_->write_single<uint8_t>(data));
        }
        __builtin_unreachable();
    }

    [[nodiscard]] Result<void, Error> read_reg(const uint8_t addr, uint8_t & data) {
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->read_reg(uint8_t(addr), data));
        }else if(spi_drv_){
            return Result<void, Error>(spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT)
            | spi_drv_->read_single<uint8_t>(data));
        }
        __builtin_unreachable();
    }

    [[nodiscard]] Result<void, Error> read_burst(const uint8_t addr, int16_t * datas, const size_t len){
        if(i2c_drv_){
            return Result<void, Error>(
                i2c_drv_->read_burst<int16_t>(uint8_t(addr), std::span(datas, len), MSB));
        }else if(spi_drv_){
            if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = spi_drv_->read_burst<int16_t>(std::span(datas, len));
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }

        __builtin_unreachable();
    }

    [[nodiscard]] Result<void, Error> write_command(const uint8_t command){
        return this->write_reg(0x7e, command);
    }
public:
    InvensenseSensor_Phy(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    InvensenseSensor_Phy(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    InvensenseSensor_Phy(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    InvensenseSensor_Phy(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    InvensenseSensor_Phy(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    InvensenseSensor_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] Result<void, Error> reset(){
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->release());
        }

        return Result<void, Error>(hal::HalResult::Ok());
    }
private:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
};
}