#include "BMI088.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;


void BMI088::init(){

}

void BMI088::update(){

}

Option<Vector3> BMI088::getAcc(){
    return Some{Vector3{0,0,0}};
}

Option<Vector3> BMI088::getGyr(){
    return Some{Vector3{0,0,0}};
}

real_t BMI088::getTemperature(){
    return 0;
}

void BMI088::writeReg(const RegAddress reg, const uint8_t data){
    if(i2c_drv_){
        i2c_drv_->writeReg((uint8_t)reg, data);
    }else if (spi_drv_){
        TODO("not implemented yet");
    }else{
        PANIC();
    }
}

void BMI088::writeReg(const RegAddress reg, uint8_t & data){
    if(i2c_drv_){
        i2c_drv_->writeReg((uint8_t)reg, data);
    }else if (spi_drv_){
        TODO("not implemented yet");
    }else{
        PANIC();
    }
}

void BMI088::requestData(const RegAddress reg_addr, int16_t * datas, const size_t len){
    if(i2c_drv_){
        i2c_drv_->readMulti((uint8_t)reg_addr, datas, len, MSB);
    }else if(spi_drv_){
        TODO("not implemented yet");
    }else{
        PANIC();
    }
}

