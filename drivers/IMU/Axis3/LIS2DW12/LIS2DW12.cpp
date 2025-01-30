#include "LIS2DW12.hpp"

using namespace ymd::drivers;

void LIS2DW12::writeReg(const uint8_t addr, const uint8_t data){
    if(i2c_drv_){
        i2c_drv_->writeReg(addr, data);
    }else if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(addr), CONT);
        spi_drv_->writeSingle(data);
    }
}

void LIS2DW12::readReg(const uint8_t addr, uint8_t & data){
    if(i2c_drv_){
        i2c_drv_->readReg((uint8_t)addr, data);
    }else if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
        spi_drv_->readSingle(data);
    }
}