#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class AsahiKaseiSensor_Phy final{
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
public:
    hal::BusError write_reg(const uint8_t addr, const uint8_t data){
        if(i2c_drv_){
            return i2c_drv_->write_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->write_single(uint8_t(addr), CONT).unwrap();
            return spi_drv_->write_single(data);
        }

        PANIC();
    }

    hal::BusError read_reg(const uint8_t addr, uint8_t & data){
        if(i2c_drv_){
            return i2c_drv_->read_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->write_single(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_single(data);
        }

        PANIC();
    }

    hal::BusError read_burst(const uint8_t addr, int16_t * datas, const size_t len){
        if(i2c_drv_){
            return i2c_drv_->read_burst<int16_t>(uint8_t(addr), std::span(datas, len), LSB);
        }else if(spi_drv_){
            spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_burst<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
        }

        PANIC();
    }

    hal::BusError verify(){
        return hal::BusError::OK;
    }
public:
    AsahiKaseiSensor_Phy(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AsahiKaseiSensor_Phy(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AsahiKaseiSensor_Phy(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr):i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    AsahiKaseiSensor_Phy(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    AsahiKaseiSensor_Phy(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    AsahiKaseiSensor_Phy(hal::Spi & spi, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv{spi, index}){;}
};
}