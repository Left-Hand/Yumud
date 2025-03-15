#pragma once

#include "IMU.hpp"

namespace ymd::drivers{

class AsahiKaseiSensor{
protected:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;

    virtual BusError writeReg(const uint8_t addr, const uint8_t data) final{
        if(i2c_drv_){
            return i2c_drv_->writeReg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->writeSingle(uint8_t(addr), CONT).unwrap();
            return spi_drv_->writeSingle(data);
        }

        PANIC();
    }

    virtual BusError readReg(const uint8_t addr, uint8_t & data) final{
        if(i2c_drv_){
            return i2c_drv_->readReg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->readSingle(data);
        }

        PANIC();
    }

    virtual BusError readBurst(const uint8_t addr, int16_t * datas, const size_t len)final{
        if(i2c_drv_){
            return i2c_drv_->readBurst<int16_t>(uint8_t(addr), std::span(datas, len), LSB);
        }else if(spi_drv_){
            spi_drv_->writeSingle<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->readBurst<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
        }

        PANIC();
    }
public:
    AsahiKaseiSensor(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AsahiKaseiSensor(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AsahiKaseiSensor(hal::I2c & i2c, const uint8_t addr):i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    AsahiKaseiSensor(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    AsahiKaseiSensor(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    AsahiKaseiSensor(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv{spi, index}){;}
};
}