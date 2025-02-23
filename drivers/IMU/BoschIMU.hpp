#pragma once

#include "IMU.hpp"

namespace ymd{

class BoschSensor{
protected:
    std::optional<I2cDrv> i2c_drv_;
    std::optional<SpiDrv> spi_drv_;

    virtual void writeReg(const uint8_t addr, const uint8_t data){
        if(i2c_drv_){
            i2c_drv_->writeReg(addr, data);
        }else if(spi_drv_){
            spi_drv_->writeSingle(uint8_t(addr), CONT);
            spi_drv_->writeSingle(data);
        }
    }

    virtual void readReg(const uint8_t addr, uint8_t & data){
        if(i2c_drv_){
            i2c_drv_->readReg((uint8_t)addr, data);
        }else if(spi_drv_){
            spi_drv_->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
            spi_drv_->readSingle(data);
        }
    }

    virtual void requestData(const uint8_t addr, int16_t * datas, const size_t len) final{
        if(i2c_drv_){
            i2c_drv_->readMulti<int16_t>(uint8_t(addr), datas, len, LSB);
        }else if(spi_drv_){
            spi_drv_->writeSingle<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT);
            spi_drv_->readMulti<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
        }
    }
public:

    BoschSensor(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    BoschSensor(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    BoschSensor(I2c & i2c, const uint8_t addr):i2c_drv_(I2cDrv{i2c, addr}){;}
    BoschSensor(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    BoschSensor(SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    BoschSensor(Spi & spi, const uint8_t index):spi_drv_(SpiDrv{spi, index}){;}
};
}