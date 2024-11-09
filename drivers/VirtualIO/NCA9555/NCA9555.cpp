#include "NCA9555.hpp"

using namespace yumud::drivers;

#define WRITE_REG(reg) writeReg(reg.address, reg);
#define READ_REG(reg) readReg(reg.address, reg);

void NCA9555::init(){
    setInversion(0);
}

void NCA9555::setInversion(const uint16_t mask){
    auto & reg = inversion_reg;
    if(mask == reg) return;
    reg = mask;
    WRITE_REG(reg);
}

void NCA9555::writePort(const uint16_t data){
    auto & reg = output_reg;
    if(data == reg) return;
    reg = data;
    WRITE_REG(reg);
}

uint16_t NCA9555::readPort(){
    auto & reg = input_reg;
    READ_REG(reg);
    return reg;
}

void NCA9555::setMode(const int index, const GpioMode mode){
    if(index > 15) PANIC()

    auto & reg = config_reg;
    uint16_t new_dir = reg;

    if(GpioUtils::isIn(mode)){
        new_dir |= (1 << index);
    }else if(GpioUtils::isOut(mode)){
        new_dir &= ~(1 << index);
    }else{
        PANIC();
    }

    if(reg != new_dir){
        WRITE_REG(reg)
    }
}