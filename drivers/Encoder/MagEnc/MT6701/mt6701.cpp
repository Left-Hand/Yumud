#include "mt6701.hpp"


using namespace ymd::drivers;
using namespace ymd;



#define MT6701_NO_I2C_FAULT\
    MT6701_DEBUG("NO I2C!!");\
    PANIC()\


void MT6701::writeReg(const RegAddress addr, const uint16_t data){
    if(i2c_drv) i2c_drv->writeReg((uint8_t)addr, data, MSB);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

void MT6701::readReg(const RegAddress addr, uint16_t & data){
    if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data, MSB);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

void MT6701::writeReg(const RegAddress addr, const uint8_t data){
    if(i2c_drv) i2c_drv->writeReg((uint8_t)addr, data);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

void MT6701::readReg(const RegAddress addr, uint8_t & data){
    if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

void MT6701::init(){
    enablePwm();
    setPwmPolarity(true);
    setPwmFreq(PwmFreq::HZ497_2);
    update();
}

void MT6701::update(){
    if(i2c_drv){
        readReg(RegAddress::RawAngle, rawAngleData);
        lap_position = u16_to_uni(rawAngleData);
    }else if(spi_drv){

        uint16_t data16;
        spi_drv->readSingle(data16);

        uint8_t data8 = 0;
        if(fast_mode == false){
            spi_drv->readSingle(data8);
        }

        semantic = Semantic{data8, data16};
        if(semantic.valid(fast_mode)){
            lap_position = real_t(iq_t<16>(semantic.data_14bit << 2) >> 16);
        } 
    }else{
        MT6701_DEBUG("no drv!!");
        PANIC();
    }
}

real_t MT6701::getLapPosition(){
    update();
    return lap_position;
}

bool MT6701::stable(){
    return semantic.valid(fast_mode);
}