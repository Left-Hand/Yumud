#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class StmicroImu_Phy final{
protected:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;

public:
    StmicroImu_Phy(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    StmicroImu_Phy(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    StmicroImu_Phy(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr):i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    StmicroImu_Phy(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    StmicroImu_Phy(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    StmicroImu_Phy(hal::Spi & spi, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv{spi, index}){;}



    [[nodiscard]] Result<void, ImuError> write_reg(const uint8_t addr, const uint8_t data) {
        if(i2c_drv_){
            if(const auto res = i2c_drv_->write_reg(uint8_t(addr), data);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }else if(spi_drv_){
            if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(addr), CONT);
                res.is_err()) return Err(res.unwrap_err());

            if(const auto res = spi_drv_->write_single<uint8_t>(data);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }

        PANIC();
    }

    [[nodiscard]] Result<void, ImuError> read_reg(const uint8_t addr, uint8_t & data) {
        if(i2c_drv_){
            if(const auto res = i2c_drv_->read_reg(uint8_t(addr), data);
                res.is_err()) return Err(res.unwrap_err());   
            return Ok();
        }else if(spi_drv_){
            if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = spi_drv_->read_single<uint8_t>(data);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }

        PANIC();
    }

    [[nodiscard]] Result<void, ImuError> read_burst(const uint8_t addr, int16_t * datas, const size_t len){
        if(i2c_drv_){
            if(const auto res = i2c_drv_->read_burst<int16_t>(uint8_t(addr), std::span(datas, len), LSB);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }else if(spi_drv_){
            if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = spi_drv_->read_burst<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }

        PANIC();
    }

    [[nodiscard]] Result<void, ImuError> validate(){
        if(i2c_drv_){
            if(const auto res = i2c_drv_->validate();
                res.is_err()) return Err(res.unwrap_err());

            return Ok();
        }

        if(spi_drv_){
            return Ok();
        }


        PANIC();
    }
};
    
}