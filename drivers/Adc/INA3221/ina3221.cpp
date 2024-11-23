#include "ina3221.hpp"

using namespace ymd::drivers;

#ifdef INA3221_DEBUG
#undef INA3221_DEBUG
#define INA3221_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA3221_PANIC(...) PANIC(__VA_ARGS__)
#define INA3221_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define INA3221_DEBUG(...)
#define INA3221_PANIC(...)  PANIC()
#define INA3221_ASSERT(cond, ...) ASSERT(cond)
#endif

#define READ_REG(reg)\
readReg(reg.address, reg);\

#define WRITE_REG(reg)\
writeReg(reg.address, reg);\

#define FAULT_IF(x)\
do{\
    if(x) PANIC()\
}while(false);\


void INA3221::init(){

}


bool INA3221::verify(){
    READ_REG(chip_id_reg);
    READ_REG(manu_id_reg);

    return (chip_id_reg.address == (chip_id_reg)) and (manu_id_reg.address == (manu_id_reg));
}

void INA3221::update(){
    requestPool(shuntvolt1_reg.address1, &shuntvolt1_reg, 6);
}
 
void INA3221::update(const size_t index){
    requestPool(shuntvolt1_reg.address1 + ((index - 1) * 2), &shuntvolt1_reg + ((index - 1) * 2), 2);
} 

void INA3221::setAverageTimes(const uint16_t times){
    uint8_t temp = CTZ(times);
    uint8_t temp2;

    if(times <= 64){
        temp2 = temp / 2;
    }else{
        temp2 = 4 + (temp - 7); 
    }

    config_reg.average_times = temp2;
    WRITE_REG(config_reg);
}

void INA3221::enableChannel(const size_t index, const bool en){
    switch(index){
        default: PANIC()
        case 1:
            config_reg.ch1_en = en;
            break;
        case 2:
            config_reg.ch2_en = en;
            break;
        case 3:
            config_reg.ch3_en = en;
            break;
    }
    WRITE_REG(config_reg);
}


void INA3221::setBusConversionTime(const ConversionTime time){
    config_reg.bus_conv_time = uint8_t(time);
    WRITE_REG(config_reg);
}


void INA3221::setShuntConversionTime(const ConversionTime time){
    config_reg.shunt_conv_time = uint8_t(time);
    WRITE_REG(config_reg);
}


void INA3221::reset(){
    config_reg.rst = true;
    WRITE_REG(config_reg);
    config_reg.rst = false;
}


int INA3221::getShuntVoltageuV(const size_t index){

    RegAddress addr;
    ShuntVoltReg & reg = [&]() -> ShuntVoltReg &{
        switch(index){
        default: PANIC()
        case 1:
            addr = shuntvolt1_reg.address1;
            return shuntvolt1_reg;
        case 2:
            addr = shuntvolt2_reg.address2;
            return shuntvolt2_reg;
        case 3:
            addr = shuntvolt3_reg.address3;
            return shuntvolt3_reg;
        }
    }();

    readReg(addr, reg);

    return reg.to_uv();
}



int INA3221::getBusVoltagemV(const size_t index){
    RegAddress addr;
    BusVoltReg & reg = [&]() -> BusVoltReg &{
        switch(index){
        default: PANIC()
        case 1:
            addr = busvolt1_reg.address1;
            return busvolt1_reg;
        case 2:
            addr = busvolt2_reg.address2;
            return busvolt2_reg;
        case 3:
            addr = busvolt3_reg.address3;
            return busvolt3_reg;
        }
    }();

    readReg(addr, reg);

    return reg.to_mv();
}


real_t INA3221::getShuntVoltage(const size_t index){
    return real_t(getShuntVoltageuV(index) / 100) / 10000;
}

real_t INA3221::getBusVoltage(const size_t index){
    return real_t(getBusVoltagemV(index)) / 1000;
}


void INA3221::setInstantOVC(const size_t index, const real_t volt){
    RegAddress addr;
    switch(index){
        default: PANIC()
        case 1:
            addr = instant_ovc1_reg.address1;
            break;
        case 2:
            addr = instant_ovc2_reg.address2;
            break;
        case 3:
            addr = instant_ovc3_reg.address3;
            break;
    }

    writeReg(addr, ShuntVoltReg::to_i16(volt));
}


void INA3221::setConstantOVC(const size_t index, const real_t volt){
    RegAddress addr;
    switch(index){
        default: PANIC()
        case 1:
            addr = constant_ovc1_reg.address1;
            break;
        case 2:
            addr = constant_ovc2_reg.address2;
            break;
        case 3:
            addr = constant_ovc3_reg.address3;
            break;
    }

    writeReg(addr, ShuntVoltReg::to_i16(volt));
}

// INA3221::INA3221Channel operator real_t(){
    
// }