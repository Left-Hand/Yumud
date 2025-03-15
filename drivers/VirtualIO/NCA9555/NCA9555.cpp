#include "NCA9555.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd::drivers;

#define WRITE_REG(reg) writeReg(reg.address, reg).unwrap();
#define READ_REG(reg) readReg(reg.address, reg).unwrap();

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

void NCA9555::setMode(const int index, const hal::GpioMode mode){
    if(index > 15) PANIC();

    auto & reg = config_reg;
    uint16_t new_dir = reg;

    if(hal::GpioUtils::isIn(mode)){
        new_dir |= (1 << index);
    }else if(hal::GpioUtils::isOut(mode)){
        new_dir &= ~(1 << index);
    }else{
        PANIC();
    }

    if(reg != new_dir){
        WRITE_REG(reg)
    }
}