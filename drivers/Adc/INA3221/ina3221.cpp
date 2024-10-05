#include "ina3221.hpp"


#define READ_REG(reg)\
readReg(reg.address, reg);\

#define WRITE_REG(reg)\
writeReg(reg.address, reg);\

void INA3221::init(){

}


bool INA3221::verify(){
    READ_REG(chip_id_reg);
    READ_REG(manu_id_reg);

    return (chip_id_reg.address == (chip_id_reg)) and (manu_id_reg.address == (manu_id_reg));
}

void INA3221::update(){
    // readReg()
} 

void INA3221::setAverageTimes(const uint16_t times){
    uint8_t temp = CTZ(times);
    uint8_t temp2;

    if(times <= 64){
        temp2 = temp / 2;
    }else{
        temp2 = 4 + (temp - 7); 
    }

    // config_reg.averageMode = temp2;
    // WRITE_REG(config_reg);
}