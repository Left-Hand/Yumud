#include "pca9685.hpp"

void PCA9685::setFrequency(uint32_t freq){
        scexpr uint32_t osc_freq = 25000000;
        prescale = osc_freq / 4096 / freq - 1;
        writeReg(RegAddress::Prescale, prescale);
    }

void PCA9685::setPwm(uint8_t channel, uint16_t on, uint16_t off){
    // writeReg(RegAddress::LED0_ON_L + 4 * channel, on & 0xFF);
    // writeReg(RegAddress::LED0_ON_H + 4 * channel, on >> 8);
    // writeReg(RegAddress::LED0_OFF_L + 4 * channel, off & 0xFF);
}

void PCA9685::init(){
    delay(10);
    // DEBUG_PRINT("m1", readReg(RegAddress::Mode1));
    // DEBUG_PRINT("m2", readReg(RegAddress::Mode2));
}


void PCA9685::reset(){
    // writeReg(RegAddress::swRst, (uint8_t)0x00);
}

void PCA9685::set(const Pin pin){
    // buf |= (uint16_t)pin;
    // write(buf);
}

void PCA9685::clr(const Pin pin){
    // buf &= ~(uint16_t)pin;
    // write(buf);
}

void PCA9685::set(const uint16_t data){
    // buf |= data;
    // write(buf);
}

void PCA9685::clr(const uint16_t data){
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

void PCA9685::setMode(const int index, const PinMode mode){
//     if(!isIndexValid(index))return;
//     uint16_t mask = 1 << index;
//     if(PinModeUtils::isIn(mode)) dir |= mask;
//     else dir &= ~mask;
//     writeReg(RegAddress::dir, dir);

//     if(index < 8){
//         ctl.p0mod = PinModeUtils::isPP(mode);
//         writeReg(RegAddress::ctl, ctl.data);
//     }
}
