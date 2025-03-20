#include "mt6701.hpp"
#include "core/debug/debug.hpp"



#ifdef MT6701_DEBUG
#undef MT6701_DEBUG
#define MT6701_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__)
#else
#define MT6701_DEBUG(...)
#endif


using namespace ymd::drivers;
using namespace ymd;



#define MT6701_NO_I2C_FAULT\
    MT6701_DEBUG("NO I2C!!");\
    PANIC()\


BusError MT6701::write_reg(const RegAddress addr, const uint16_t data){
    if(i2c_drv) return i2c_drv->write_reg(uint8_t(addr), data, MSB);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

BusError MT6701::read_reg(const RegAddress addr, uint16_t & data){
    if(i2c_drv) return i2c_drv->read_reg(uint8_t(addr), data, MSB);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

BusError MT6701::write_reg(const RegAddress addr, const uint8_t data){
    if(i2c_drv) return i2c_drv->write_reg(uint8_t(addr), data);
    else{
        MT6701_NO_I2C_FAULT;
    }
}

BusError MT6701::read_reg(const RegAddress addr, uint8_t & data){
    if(i2c_drv) return i2c_drv->read_reg(uint8_t(addr), data);
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
        read_reg(RegAddress::RawAngle, rawAngleData);
        lap_position = u16_to_uni(rawAngleData);
    }else if(spi_drv){

        uint16_t data16;
        spi_drv->readSingle(data16).unwrap();

        uint8_t data8 = 0;
        if(fast_mode == false){
            spi_drv->readSingle(data8).unwrap();
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