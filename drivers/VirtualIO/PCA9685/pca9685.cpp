#include "pca9685.hpp"
#include "sys/debug/debug.hpp"


using namespace ymd;
using namespace ymd::drivers;


void PCA9685::setFrequency(uint freq, real_t trim){
        readReg(RegAddress::Mode1, mode1_reg);
        
        prescale_reg = int((real_t(25000000.0 / 4096) / freq - 1) * trim);
        // prescale_reg = 121;
        // enableSleep();
        mode1_reg.sleep = true;
        writeReg(RegAddress::Mode1, mode1_reg);
        writeReg(RegAddress::Prescale, prescale_reg);
        mode1_reg.sleep = false;
        
        writeReg(RegAddress::Mode1, mode1_reg);
        // enableSleep(false);
        delay(5);
        mode1_reg = uint8_t(mode1_reg | uint8_t(0xa1));
        writeReg(RegAddress::Mode1, mode1_reg);
    }

void PCA9685::setPwm(uint8_t channel, uint16_t on, uint16_t off){
    if(channel > 15) PANIC();
    
    if(sub_channels[channel].on.cvr != on){
        writeReg(RegAddress(uint8_t(RegAddress::LED0_ON_L) + 4 * channel), on);
        sub_channels[channel].on.cvr = on;
    }

    if(sub_channels[channel].off.cvr != off){
        auto & reg = sub_channels[channel].off;

        // DEBUG_PRINTLN(off);
        // if(off >= (4096)){
            // reg.full = true;   
            // DEBUG_PRINTLN(off);
        // }else if(sub_channels[channel].off.cvr != off){
            reg.full = false;
            reg.cvr = off;
        // }
        writeReg(RegAddress(uint8_t(RegAddress::LED0_OFF_L) + 4 * channel), reg);
    }
}

void PCA9685::init(){
    mode1_reg = 0;
    writeReg(RegAddress::Mode1, mode1_reg);
    for(size_t i = 0; i < 16; i++){
        setPwm(i, 0, 0);
    }
    delay(10);
}


void PCA9685::reset(){
    readReg(RegAddress::Mode1, mode1_reg);
    if(1 == mode1_reg.restart){
        mode1_reg.sleep = 0;
        writeReg(RegAddress::Mode1, mode1_reg);
    }
    delayMicroseconds(500);
    mode1_reg.restart = 1;
    writeReg(RegAddress::Mode1, mode1_reg);
    mode1_reg.restart = 0;
}

void PCA9685::enableExtClk(const bool en){
    mode1_reg.extclk = en;
    writeReg(RegAddress::Mode1, mode1_reg);
}

void PCA9685::enableSleep(const bool en){
    mode1_reg.sleep = en;
    writeReg(RegAddress::Mode1, mode1_reg);
    mode1_reg.sleep = 0;
}

void PCA9685::setPin(const uint16_t data){
    // buf |= data;
    // write(buf);
}

void PCA9685::clrPin(const uint16_t data){
    // buf &= ~data;
    // write(buf);
}

void PCA9685::writeByIndex(const int index, const bool data){
    // if(!isIndexValid(index))return;
    // if(data) buf |= 1 << index;
    // else buf &= ~(1 << index);
    // write(buf);
}

bool PCA9685::readByIndex(const int index){
    // if(!isIndexValid(index)) return false;
    // read();
    // return (buf & (1 << index));
    return true;
}

void PCA9685::setMode(const int index, const hal::GpioMode mode){
//     if(!isIndexValid(index))return;
//     uint16_t mask = 1 << index;
//     if(GpioMode::isIn(mode)) dir |= mask;
//     else dir &= ~mask;
//     writeReg(RegAddress::dir, dir);

//     if(index < 8){
//         ctl.p0mod = GpioMode::isPP(mode);
//         writeReg(RegAddress::ctl, ctl.data);
//     }
}

void PCA9685::setSubAddr(const uint8_t index, const uint8_t addr){
    sub_addr_regs[index] = addr;
    writeReg(RegAddress(uint8_t(RegAddress::SubAddr) + index), sub_addr_regs[index]);
}
