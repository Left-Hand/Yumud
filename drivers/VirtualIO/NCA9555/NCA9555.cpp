#include "NCA9555.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;

#define WRITE_REG(reg) write_reg(reg.address, reg);
#define READ_REG(reg) read_reg(reg.address, reg);

void NCA9555::init(){
    set_inversion(0);
}

void NCA9555::set_inversion(const uint16_t mask){
    auto & reg = inversion_reg;
    if(mask == reg) return;
    reg = mask;
    WRITE_REG(reg);
}

void NCA9555::write_port(const uint16_t data){
    auto & reg = output_reg;
    if(data == reg) return;
    reg = data;
    WRITE_REG(reg);
}

uint16_t NCA9555::read_port(){
    auto & reg = input_reg;
    READ_REG(reg);
    return reg;
}

void NCA9555::set_mode(const int index, const hal::GpioMode mode){
    if(index > 15) PANIC();

    auto & reg = config_reg;
    uint16_t new_dir = reg;

    if(mode.is_in_mode()){
        new_dir |= (1 << index);
    }else if(mode.is_out_mode()){
        new_dir &= ~(1 << index);
    }else{
        PANIC();
    }

    if(reg != new_dir){
        WRITE_REG(reg)
    }
}