#pragma once

#include "IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class AnalogDeviceIMU{
private:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
protected:
    BusError write_reg(const uint8_t reg_address, const uint8_t reg_data){
        if(i2c_drv_){
            return i2c_drv_->write_reg((uint8_t)((uint8_t)reg_address & 0x7F), reg_data);
        } else if (spi_drv_) {
            TODO("spi is not support yet");
        }
    }
    
    BusError read_reg(const uint8_t reg_address, uint8_t & reg_data){
        if(i2c_drv_){
            return i2c_drv_->read_reg(uint8_t(reg_address), reg_data);
        }else if(spi_drv_){
            TODO("spi is not support yet");
        }
    }
public:
    AnalogDeviceIMU(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AnalogDeviceIMU(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AnalogDeviceIMU(hal::I2c & i2c, const uint8_t addr):i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    AnalogDeviceIMU(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    AnalogDeviceIMU(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    AnalogDeviceIMU(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv{spi, index}){;}
};
}