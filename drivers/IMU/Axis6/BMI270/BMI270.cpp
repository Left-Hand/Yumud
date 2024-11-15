#include "BMI270.hpp"

using namespace ymd::drivers;

void BMI270::writeReg(const uint8_t addr, const uint8_t data){
    if(i2c_drv_) i2c_drv_->writeReg(addr, data, MSB);
    if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(addr), CONT);
        spi_drv_->writeSingle(data);

        BMI270_DEBUG("Wspi", addr, data);

    }
}

void BMI270::readReg(const RegAddress addr, uint8_t & data){
    if(i2c_drv_) i2c_drv_->readReg((uint8_t)addr, data, MSB);
    if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
        spi_drv_->readSingle(data);
    }

    BMI270_DEBUG("Rspi", addr, data);
}

void BMI270::requestData(const RegAddress addr, void * datas, const size_t len){
    if(i2c_drv_) i2c_drv_->readMulti(uint8_t(addr), (uint8_t *)datas, len, MSB);
    if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
        spi_drv_->readMulti((uint8_t *)(datas), len);
    }

    BMI270_DEBUG("Rspi", addr, len);
}
