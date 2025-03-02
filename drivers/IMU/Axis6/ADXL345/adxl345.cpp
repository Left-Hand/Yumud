#include "adxl345.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd::drivers;

void ADXL345::writeReg(const RegAddress reg_address, const uint8_t reg_data){
    if(i2c_drv){
        i2c_drv->writeReg((uint8_t)((uint8_t)reg_address & 0x7F), reg_data);
    } else if (spi_drv) {
        TODO("spi is not support yet");
    }
}

void ADXL345::readReg(const RegAddress reg_address, uint8_t & reg_data){
    if(i2c_drv){
        i2c_drv->readReg(uint8_t(reg_address), reg_data);
    }else if(spi_drv){
        TODO("spi is not support yet");
    }
}