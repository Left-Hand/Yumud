#include "aw9523.hpp"

void AW9523::setLedCurrentLimit(const CurrentLimit limit){
    ctl.isel = (uint8_t)limit;
    writeReg(RegAddress::ctl, ctl.data);
}

void AW9523::setLedCurrent(const Pin pin, const uint8_t current){
    uint index = CTZ((uint16_t)pin);
    if(index < 8) index += 4;
    else if(index < 12) index -= 8;
    writeReg((RegAddress)((uint8_t)RegAddress::dim + index), current);
}
