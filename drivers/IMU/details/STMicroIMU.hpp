#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class STMicroSensor{
protected:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;

    virtual BusError write_reg(const uint8_t addr, const uint8_t data) final{
        if(i2c_drv_){
            return i2c_drv_->write_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->write_single(uint8_t(addr), CONT).unwrap();
            return spi_drv_->write_single(data);
        }

        PANIC();
    }

    virtual BusError read_reg(const uint8_t addr, uint8_t & data) final{
        if(i2c_drv_){
            return i2c_drv_->read_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->write_single(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_single(data);
        }

        PANIC();
    }

    virtual BusError read_burst(const uint8_t addr, int16_t * datas, const size_t len)final{
        if(i2c_drv_){
            return i2c_drv_->read_burst<int16_t>(uint8_t(addr), std::span(datas, len), LSB);
        }else if(spi_drv_){
            spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_burst<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
        }

        PANIC();
    }
public:
    STMicroSensor(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    STMicroSensor(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    STMicroSensor(hal::I2c & i2c, const uint8_t addr):i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    STMicroSensor(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    STMicroSensor(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    STMicroSensor(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv{spi, index}){;}
};


class ImuPhyStmicro final{
protected:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;

public:
    ImuPhyStmicro(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    ImuPhyStmicro(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    ImuPhyStmicro(hal::I2c & i2c, const uint8_t addr):i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    ImuPhyStmicro(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    ImuPhyStmicro(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    ImuPhyStmicro(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv{spi, index}){;}



    BusError write_reg(const uint8_t addr, const uint8_t data) {
        if(i2c_drv_){
            return i2c_drv_->write_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->write_single(uint8_t(addr), CONT).unwrap();
            return spi_drv_->write_single(data);
        }

        PANIC();
    }

    BusError read_reg(const uint8_t addr, uint8_t & data) {
        if(i2c_drv_){
            return i2c_drv_->read_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->write_single(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_single(data);
        }

        PANIC();
    }

    BusError read_burst(const uint8_t addr, int16_t * datas, const size_t len){
        if(i2c_drv_){
            return i2c_drv_->read_burst<int16_t>(uint8_t(addr), std::span(datas, len), LSB);
        }else if(spi_drv_){
            spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_burst<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
        }

        PANIC();
    }

    BusError verify(){
        if(i2c_drv_) return i2c_drv_->verify();
        if(spi_drv_) return BusError::OK;
        PANIC();
    }
};
    
}