#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class AnalogDeviceIMU_Phy final{
private:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
    
public:
    using Error = ImuError;

    AnalogDeviceIMU_Phy(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AnalogDeviceIMU_Phy(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AnalogDeviceIMU_Phy(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    AnalogDeviceIMU_Phy(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    AnalogDeviceIMU_Phy(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    AnalogDeviceIMU_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):spi_drv_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] Result<void, ImuError> write_reg(const uint8_t reg_addr, const uint8_t reg_data){
        if(i2c_drv_){
            if(const auto res = i2c_drv_->write_reg((uint8_t)((uint8_t)reg_addr & 0x7F), reg_data);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        } else if (spi_drv_) {
            return Err(Error::SpiPhyIsNotImplementedYet);
        }
        __builtin_unreachable();
    }
    
    [[nodiscard]] Result<void, ImuError> read_reg(const uint8_t reg_addr, uint8_t & reg_data){
        if(i2c_drv_){
            if(const auto res = i2c_drv_->read_reg(uint8_t(reg_addr), reg_data);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }else if(spi_drv_){
            return Err(Error::SpiPhyIsNotImplementedYet);
        }
        __builtin_unreachable();
    }

    [[nodiscard]] Result<void, ImuError> read_burst(const uint8_t addr, std::span<int16_t> pbuf){
        if(i2c_drv_){
            if(const auto res = i2c_drv_->read_burst<int16_t>(uint8_t(addr), pbuf, LSB);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }else if(spi_drv_){
            if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = spi_drv_->read_burst<int16_t>(pbuf);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }

        return Err(ImuError::NoAvailablePhy);
    }
};
}