#include "mt6701.hpp"


using namespace yumud::drivers;
using namespace yumud;



#define MT6701_NO_I2C_FAULT\
    MT6701_DEBUG("NO I2C!!");\
    CREATE_FAULT\


void MT6701::writeReg(const RegAddress regAddress, const uint16_t regData){
    if(i2c_drv) i2c_drv->writeReg((uint8_t)regAddress, regData);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

void MT6701::readReg(const RegAddress regAddress, uint16_t & regData){
    if(i2c_drv) i2c_drv->readReg((uint8_t)regAddress, regData);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

void MT6701::writeReg(const RegAddress regAddress, const uint8_t regData){
    if(i2c_drv) i2c_drv->writeReg((uint8_t)regAddress, regData);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

void MT6701::readReg(const RegAddress regAddress, uint8_t & regData){
    if(i2c_drv) i2c_drv->readReg((uint8_t)regAddress, regData);
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
        u16_to_uni(rawAngleData, lap_position);
    }else if(spi_drv){

        uint16_t data16;
        spi_drv->read(data16);

        uint8_t data8 = 0;
        if(fast_mode == false){
            spi_drv->read(data8);
        }

        semantic = Semantic{data8, data16};
        if(semantic.valid(fast_mode)){
            u16_to_uni(semantic.data_14bit << 2, lap_position);
        } 
    }else{
        MT6701_DEBUG("no drv!!");
        CREATE_FAULT;
    }
}

real_t MT6701::getLapPosition(){
    update();
    return lap_position;
}

bool MT6701::stable(){
    return semantic.valid(fast_mode);
}