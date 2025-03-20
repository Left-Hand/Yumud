#pragma once

#include "IMU.hpp"

namespace ymd::drivers{

class InvensenseSensor{
protected:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;

    virtual BusError write_reg(const uint8_t addr, const uint8_t data) final{
        if(i2c_drv_){
            return i2c_drv_->write_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->writeSingle(uint8_t(addr), CONT).unwrap();
            return spi_drv_->writeSingle(data);
        }

        PANIC();
    }

    virtual BusError read_reg(const uint8_t addr, uint8_t & data) final{
        if(i2c_drv_){
            return i2c_drv_->read_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->readSingle(data);
        }

        PANIC();
    }

    virtual BusError read_burst(const uint8_t addr, int16_t * datas, const size_t len)final{
        if(i2c_drv_){
            return i2c_drv_->read_burst<int16_t>(uint8_t(addr), std::span(datas, len), LSB);
        }else if(spi_drv_){
            spi_drv_->writeSingle<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_burst<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
        }

        PANIC();
    }
public:
    InvensenseSensor(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    InvensenseSensor(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    InvensenseSensor(hal::I2c & i2c, const uint8_t addr):i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    InvensenseSensor(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    InvensenseSensor(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    InvensenseSensor(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv{spi, index}){;}
};
}